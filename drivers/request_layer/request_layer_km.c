//
// Created by lukemartinlogan on 5/6/21.
//

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

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("request_layer_km");

//Macros
#define BDEV_ACCESS_FLAGS FMODE_READ | FMODE_WRITE | FMODE_PREAD | FMODE_PWRITE //| FMODE_EXCL

void *virt_mem;

//Prototypes
static int __init init_request_layer_km(void);
static void __exit exit_request_layer_km(void);

//Helpers
static void io_complete(struct bio *bio)
{
    printk("request_layer_km: Request complete!");
}

static inline struct bio *create_bio(struct block_device *bdev, struct page **pages, int num_pages, size_t sector, int op)
{
    struct bio *bio;
    int i;

    bio = bio_alloc(GFP_KERNEL, num_pages);
    if(bio == NULL) {
        printk(KERN_INFO "create_bio: Could not allocate bio request (%ld bytes)\n", sizeof(struct bio)+num_pages*sizeof(struct bio_vec));
        return NULL;
    }
    bio_set_dev(bio, bdev);
    bio_set_op_attrs(bio, op, 0);
    bio_set_flag(bio, BIO_USER_MAPPED);
    bio->bi_iter.bi_sector = sector;
    bio->bi_end_io = &io_complete;
    for(i = 0; i < num_pages; ++i) {
        bio_add_page(bio, pages[i], PAGE_SIZE, 0);
    }
    bio->bi_private = NULL;
    bio->bi_status = BLK_STS_OK;

    return bio;
}

//Convert bio to request
void bio_to_rq(struct request *rq, struct bio *bio, unsigned int nr_segs)
{
    rq->__sector = bio->bi_iter.bi_sector;
    rq->write_hint = bio->bi_write_hint;
    rq->nr_phys_segments = nr_segs;
    rq->__data_len = bio->bi_iter.bi_size;
    rq->bio = rq->biotail = bio;
    rq->ioprio = bio_prio(bio);
    rq->rq_disk = bio->bi_disk;
}

//Dispatch request
bool dispatch_rq(struct blk_mq_hw_ctx *hctx, struct request *rq)
{
    struct request_queue *q = hctx->queue;
    blk_status_t ret = BLK_STS_OK;
    struct blk_mq_queue_data bd;

    bd.rq = rq;
    bd.last = true;
    printk("request_layer_km: %p", q->mq_ops->queue_rq);
    ret = q->mq_ops->queue_rq(hctx, &bd);
    switch (ret) {
        case BLK_STS_OK:
            printk("request_layer_km: Request dispatched!\n");
            return true;
        default:
            printk("request_layer_km: Request failed!\n");
            return false;
    }
    return false;
}

static int __init init_request_layer_km(void)
{
    char *dev = "/dev/sdb";
    struct block_device *bdev;
    struct request_queue *queue;
    struct blk_mq_hw_ctx **hctxs, *hctx;
    int nr_hw_queues;
    struct bio *bio;
    struct request *rq;
    struct page *page;
    bool ret;

    //Allocate some kmem
    virt_mem = kmalloc(4096, GFP_KERNEL | GFP_DMA);
    memset(virt_mem, 0x8, 4096);
    printk("request_layer_km: Created VA %p\n", virt_mem);
    //Convert to pages
    page = vmalloc_to_page(virt_mem);
    printk("request_layer_km: Created page %p\n", page);
    //Get block device associated with semantic label
    bdev = blkdev_get_by_path(dev, BDEV_ACCESS_FLAGS, NULL);
    printk("request_layer_km: Acquried bdev %p\n", bdev);
    //Get request queue associated with device
    queue = bdev->bd_disk->queue;
    printk("request_layer_km: Queue %p\n", queue);
    //Create bio
    bio = create_bio(bdev, &page, 1, 0, REQ_OP_WRITE);
    printk("request_layer_km: Create bio %p\n", bio);
    //Get HW queues associated with bdev
    hctxs = queue->queue_hw_ctx;
    printk("request_layer_km: Get hctxs %p\n", hctxs);
    nr_hw_queues = queue->nr_hw_queues;
    printk("request_layer_km: nr_hw_queues %d\n", nr_hw_queues);
    hctx = hctxs[0];
    printk("request_layer_km: hctx %p\n", hctx);
    //Create request to hctx
    rq = kmalloc(sizeof(struct request), GFP_KERNEL);
    printk("request_layer_km: request %p\n", rq);
    bio_to_rq(rq, bio, 1);
    printk("request_layer_km: bio_to_rq\n");
    //Execute request
    ret = dispatch_rq(hctx, rq);
    printk("request_layer_km: ret %d\n", ret);
    /*
    //Insert a request into a HW queue dispatch list
    spin_lock(&hctx->lock);
    list_add(&rq->queuelist, &hctx->dispatch);
    spin_unlock(&hctx->lock);
    //Run the HW queue
    blk_mq_run_hw_queue(hctx, false);*/
    //Free kmem, bio, request
    return 0;
}



static void __exit exit_request_layer_km(void)
{
}

module_init(init_request_layer_km)
module_exit(exit_request_layer_km)
