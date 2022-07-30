
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/bio.h>
#include <linux/blk_types.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/list.h>
#include <linux/cpumask.h>
#include <linux/timer.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/file.h>
#include <linux/namei.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("bio_layer_km");

//Macros
#define NETLINK_USER 31
#define MAX_DEVNAME 32
#define MAX_MOUNTED_BDEVS 32
#define BDEV_ACCESS_FLAGS FMODE_READ | FMODE_WRITE | FMODE_PREAD | FMODE_PWRITE //| FMODE_EXCL
#define BIO_MAX_PAGES_ALLOC 256
#define PAGE_TO_SECTOR (PAGE_SIZE >> 9)

//Data definitions
struct dev_data {
    int is_active;
    char dev_name[MAX_DEVNAME];
    struct block_device *bdev;
};
struct bio_priv_data {
    struct bio *next_bio;
    ktime_t end_time;
    struct completion done;
};
struct labstor_km_request {
    int code;
    char dev_name[MAX_DEVNAME];
    size_t sector;
    void *usr_buf;
    size_t length;
    unsigned long long time_ns;
};
int queue_tail = 0;
static struct dev_data device_list[MAX_MOUNTED_BDEVS] = {0};
struct sock *nl_sk = NULL;

//Prototypes
static int __init init_linux_driver_io(void);
static int start_server(void);
static void server_loop(struct sk_buff *skb);
static struct dev_data *alloc_block_device(char *dev);
static struct dev_data *find_block_device(char *dev);
void release_all_block_devices(void);
static void send_msg_to_usr(int code, ktime_t time_ns, int pid);
static void io_complete(struct bio *bio);
static inline struct page **convert_usr_buf(void *usr_buf, size_t length, int *num_pagesp);
static inline struct bio *create_bio(struct dev_data *dd, struct page **pages, int num_pages, size_t sector, int op);
static inline struct bio **create_bios(struct dev_data *dd, struct page **pages, int num_pages, size_t sector, int op, int *num_biosp);
static inline int submit_bios(struct bio **bios, int num_bios);
static inline int wait_for_bios(struct bio **bios, int num_bios);
static inline ktime_t get_end_time(struct bio **bios, int num_bios);
static inline void free_bios(struct bio **bios, int num_bios);
static void mount_device(char *dev, int pid);
static void io_bypass(char *dev, size_t sector, void *usr_buf, size_t length, int pid, int op);
static void __exit exit_linux_driver_io(void);


//Helpers
static int __init init_linux_driver_io(void)
{
    start_server();
    return 0;
}

static int start_server(void)
{
    struct netlink_kernel_cfg cfg = {
            .input = server_loop,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if(!nl_sk)
    {
        printk(KERN_ALERT "time_linux_driver_io_km: Error creating socket.\n");
        return -10;
    }
    printk(KERN_INFO "time_linux_driver_io_km: Netlink socket initialized");
    return 0;
}

static void server_loop(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    struct labstor_km_request *rq;
    int pid;

    nlh=(struct nlmsghdr*)skb->data;
    rq = (struct labstor_km_request*)nlmsg_data(nlh);
    pid = nlh->nlmsg_pid; /*pid of sending process */

    switch(rq->code) {
        case 1: {
            mount_device(rq->dev_name, pid);
            break;
        }

        case 2: {
            io_bypass(rq->dev_name, rq->sector, rq->usr_buf, rq->length, pid, REQ_OP_WRITE);
            break;
        }

        case 3: {
            io_bypass(rq->dev_name, rq->sector, rq->usr_buf, rq->length, pid, REQ_OP_READ);
            break;
        }
    }
}

static struct dev_data *alloc_block_device(char *dev)
{
    struct dev_data *dd;
    int i = 0;

    for(i = 0; i < MAX_MOUNTED_BDEVS; i++) {
        dd = device_list + (queue_tail + i)%32;
        if(!dd->is_active) {
            strcpy(dd->dev_name, dev);
            queue_tail = (queue_tail + 1)%32;
            return dd;
        }
    }

    return NULL;
}

static struct dev_data *find_block_device(char *dev)
{
    int i = 0;

    for(i = 0; i < MAX_MOUNTED_BDEVS; i++) {
        if(device_list[i].is_active) {
            if (strncmp(dev, device_list[i].dev_name, MAX_DEVNAME) == 0) {
                return device_list + i;
            }
        }
    }

    return NULL;
}

void release_all_block_devices(void)
{
    struct dev_data *dd;
    int i = 0;

    for(i = 0; i < MAX_MOUNTED_BDEVS; i++) {
        dd = device_list + i;
        if(dd->is_active) {
            blkdev_put(dd->bdev, BDEV_ACCESS_FLAGS);
            dd->is_active = 0;
        }
    }
}

static void send_msg_to_usr(int code, ktime_t time_ns, int pid)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int res = 0;
    struct labstor_km_request *rq;

    skb_out = nlmsg_new(sizeof(struct labstor_km_request), 0);
    if(!skb_out) {
        printk(KERN_ERR "time_linux_driver_io_km: Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, sizeof(struct labstor_km_request), 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    rq = nlmsg_data(nlh);
    rq->code = code;
    rq->time_ns = time_ns;
    res=nlmsg_unicast(nl_sk, skb_out, pid);
    if(res<0) {
        printk(KERN_ERR "time_linux_driver_io_km: Error while sending back to user\n");
    }
}

static void io_complete(struct bio *bio)
{
    struct bio_priv_data *priv = bio->bi_private;
    priv->end_time = ktime_get_ns();
    complete(&priv->done);
}

static inline struct page **convert_usr_buf(void *usr_buf, size_t length, int *num_pagesp)
{
    struct page **pages;
    int num_pages;

    num_pages = length/PAGE_SIZE;
    pages = kmalloc(num_pages*sizeof(struct page *), GFP_KERNEL);
    if(pages == NULL) {
        printk(KERN_INFO "time_linux_driver_io_km: Could not allocate space for %d pages\n", num_pages);
        return NULL;
    }
    get_user_pages_fast((long unsigned)usr_buf, num_pages, 0, pages);

    *num_pagesp = num_pages;
    return pages;
}

static inline struct bio *create_bio(struct dev_data *dd, struct page **pages, int num_pages, size_t sector, int op)
{
    struct bio *bio;
    struct bio_priv_data *priv;
    int i;

    bio = bio_alloc(GFP_KERNEL, num_pages);
    if(bio == NULL) {
        printk(KERN_INFO "time_linux_driver_io_km: Could not allocate bio request (%ld bytes)\n", sizeof(struct bio)+num_pages*sizeof(struct bio_vec));
        return NULL;
    }
    bio_set_dev(bio, dd->bdev);
    //bio_set_op_attrs(bio, op, REQ_SYNC);
    bio_set_op_attrs(bio, op, 0);
    bio_set_flag(bio, BIO_USER_MAPPED);
    bio->bi_iter.bi_sector = sector;
    bio->bi_end_io = &io_complete;
    for(i = 0; i < num_pages; ++i) {
        bio_add_page(bio, pages[i], PAGE_SIZE, 0);
    }
    priv = kmalloc(sizeof(struct bio_priv_data), GFP_KERNEL);
    if(priv == NULL) {
        printk(KERN_INFO "time_linux_driver_io_km: Could not allocate private data\n");
        bio_put(bio);
        return NULL;
    }
    priv->done = COMPLETION_INITIALIZER_ONSTACK_MAP(priv->done, bio->bi_disk->lockdep_map);
    bio->bi_private = priv;
    bio->bi_status = BLK_STS_OK;

    return bio;
}

static inline struct bio **create_bios(struct dev_data *dd, struct page **pages, int num_pages, size_t sector, int op, int *num_biosp)
{
    struct bio **bios;
    int i, num_bios, niter;
    struct bio_priv_data *priv;
    int remainder;

    remainder = (num_pages%BIO_MAX_PAGES_ALLOC);
    num_bios = remainder ? (num_pages/BIO_MAX_PAGES_ALLOC) + 1: (num_pages/BIO_MAX_PAGES_ALLOC);
    niter = num_bios - (remainder!=0);
    bios = kmalloc(num_bios*sizeof(struct bio *), GFP_KERNEL);
    if(bios == NULL) {
        printk(KERN_INFO "time_linux_driver_io_km: Could not allocate multiple bios (%ld bytes)\n", num_bios * sizeof(struct bio *));
        return NULL;
    }

    for(i = 0; i < niter; ++i) {
        bios[i] = create_bio(dd, pages + i*BIO_MAX_PAGES_ALLOC, BIO_MAX_PAGES_ALLOC, sector + i*BIO_MAX_PAGES_ALLOC*PAGE_TO_SECTOR, op);
        if(bios[i] == NULL) {
            free_bios(bios, i);
            return NULL;
        }
        priv = bios[i]->bi_private;
        priv->next_bio = NULL;
        if(i > 0) {
            priv = bios[i-1]->bi_private;
            priv->next_bio = bios[i];
        }
    }
    if(remainder) {
        bios[i] = create_bio(dd, pages + i*BIO_MAX_PAGES_ALLOC, remainder, sector + i*BIO_MAX_PAGES_ALLOC*PAGE_TO_SECTOR, op);
        if(bios[i] == NULL) {
            free_bios(bios, num_bios);
            return NULL;
        }
        priv = bios[i]->bi_private;
        priv->next_bio = NULL;
        if(i > 0) {
            priv = bios[i-1]->bi_private;
            priv->next_bio = bios[i];
        }
    }

    *num_biosp = num_bios;
    return bios;
}

static inline int submit_bios(struct bio **bios, int num_bios)
{
    int i = 0;
    struct blk_plug plug;

    blk_start_plug(&plug);
    for(i = 0; i < num_bios; ++i) {
        submit_bio(bios[i]);
    }
    blk_finish_plug(&plug);
    return 1;
}

static inline int wait_for_bios(struct bio **bios, int num_bios)
{
    int i;
    struct bio_priv_data *priv;

    for(i = 0; i < num_bios; ++i) {
        priv = bios[i]->bi_private;
        wait_for_completion_io(&priv->done);
        if(bios[i]->bi_status != BLK_STS_OK) {
            printk("time_linux_driver_io_km: bio_status: %d\n", blk_status_to_errno(bios[i]->bi_status));
            return 0;
        }
    }

    return 1;
}

static inline ktime_t get_end_time(struct bio **bios, int num_bios)
{
    int i;
    struct bio_priv_data *priv;
    ktime_t end_time = 0;

    for(i = 0; i < num_bios; ++i) {
        priv = bios[i]->bi_private;
        if(priv->end_time > end_time) {
            end_time = priv->end_time;
        }
    }

    return end_time;
}

static inline void free_bios(struct bio **bios, int num_bios)
{
    int i;

    for(i = 0; i < num_bios; ++i) {
        if(bios[i] != NULL) {
            if(bios[i]->bi_private) {
                kfree(bios[i]->bi_private);
            }
            bio_put(bios[i]);
        } else {
            break;
        }
    }

    kfree(bios);
}

//Main calls
static void mount_device(char *dev, int pid)
{
    //Acquire a free block device structure
    struct dev_data *dd = alloc_block_device(dev);

    //Acquire struct block_dev
    dd->bdev = blkdev_get_by_path(dev, BDEV_ACCESS_FLAGS, NULL);
    if (IS_ERR(dd->bdev)) {
        printk(KERN_INFO "time_linux_driver_io_km: can't open %s <%lu>\n", dev, PTR_ERR(dd->bdev));
        send_msg_to_usr(-1, 0, pid);
        dd->is_active = 0;
        return;
    }
    dd->is_active = 1;
    printk(KERN_INFO "time_linux_driver_io_km: %s is mounted!\n", dev);

    //Send return code back to user
    send_msg_to_usr(0, 0, pid);
    return;
}

static void io_bypass(char *dev, size_t sector, void *usr_buf, size_t length, int pid, int op)
{
    struct dev_data *dd;
    struct page **pages;
    struct bio **bios;
    ktime_t start_time, end_time;
    int num_pages, num_bios;

    //Find block device
    dd = find_block_device(dev);
    if(dd == NULL) {
        printk(KERN_INFO "time_linux_driver_io_km: Could not find block device %s\n", dev);
        send_msg_to_usr(-1, 0, pid);
        return;
    }

    //Convert user buffer to physical pages
    pages = convert_usr_buf(usr_buf, length, &num_pages);
    if(pages == NULL) {
        send_msg_to_usr(-1, 0, pid);
        return;
    }

    //Create bios
    bios = create_bios(dd, pages, num_pages, sector, op, &num_bios);
    if(bios == NULL) {
        kfree(pages);
        send_msg_to_usr(-1, 0, pid);
        return;
    }

    //Get start time
    start_time = ktime_get_ns();

    //Submit bios
    if(!submit_bios(bios, num_bios)) {
        kfree(pages);
        free_bios(bios, num_bios);
        send_msg_to_usr(-1, 0, pid);
        return;
    }

    //Wait for all bios to complete
    if(!wait_for_bios(bios, num_bios)) {
        kfree(pages);
        free_bios(bios, num_bios);
        send_msg_to_usr(-1, 0, pid);
        return;
    }

    //Get end time
    end_time = get_end_time(bios, num_bios);

    //Send completion to user
    send_msg_to_usr(0, end_time - start_time, pid);

    //Release data
    kfree(pages);
    free_bios(bios, num_bios);
}

static void __exit exit_linux_driver_io(void)
{
    release_all_block_devices();
    netlink_kernel_release(nl_sk);
    printk(KERN_INFO "time_linux_driver_io_km: Module has been removed!\n");
}

module_init(init_linux_driver_io)
module_exit(exit_linux_driver_io)