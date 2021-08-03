//
// Created by lukemartinlogan on 5/6/21.
//

/*
 * A kernel module that constructs bio and request objects, and submits them to the underlying drivers.
 * The request gets submitted, but can't read from device afterwards...
 * But the I/O completes?
 * */

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
#include <linux/delay.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("request_layer_km");

/*struct blk_mq_tags {
    unsigned int nr_tags;
    unsigned int nr_reserved_tags;

    atomic_t active_queues;

    struct sbitmap_queue bitmap_tags;
    struct sbitmap_queue breserved_tags;

    struct request **rqs;
    struct request **static_rqs;
    struct list_head page_list;
};*/

//Macros
#define BDEV_ACCESS_FLAGS FMODE_READ | FMODE_WRITE | FMODE_PREAD | FMODE_PWRITE //| FMODE_EXCL

void *virt_mem;
struct request_queue *queue;

//Prototypes
static int __init init_request_layer_km(void);
static void __exit exit_request_layer_km(void);

/**
 * I/O REQUEST FUNCTIONS
 * */

static void req_complete(struct request *rq, blk_status_t error)
{
    printk("request_layer_km: REQ Request complete (%d)!\n", error);
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
    //bio_set_op_attrs(bio, op, 0);
    bio->bi_opf = op;
    //bio_set_flag(bio, BIO_USER_MAPPED);
    //bio->bi_flags |= (1U << BIO_USER_MAPPED);
    bio->bi_iter.bi_sector = sector;
    bio->bi_end_io = NULL;
    for(i = 0; i < num_pages; ++i) {
        bio_add_page(bio, pages[i], PAGE_SIZE, 0);
    }
    bio->bi_private = NULL;
    bio->bi_status = BLK_STS_OK;
    bio->bi_ioprio = 0;

    //bio_integrity_prep(bio);

    return bio;
}

struct request *bio_to_rq(struct request_queue *q, struct blk_mq_hw_ctx *hctx, struct bio *bio, unsigned int nr_segs)
{
    struct request *rq = blk_mq_alloc_request_hctx(q, REQ_OP_WRITE, BLK_MQ_REQ_NOWAIT, 0);

    rq->end_io = req_complete;
    rq->nr_phys_segments = nr_segs;
    rq->__data_len = bio->bi_iter.bi_size;
    rq->bio = rq->biotail = bio;
    rq->ioprio = bio->bi_ioprio;
    rq->rq_disk = bio->bi_disk;

    rq->rq_flags = 0;
    rq->cmd_flags = bio->bi_opf;
    rq->part = NULL;
    rq->io_start_time_ns = 0;
    rq->stats_sectors = 0;
    rq->end_io = req_complete;
    rq->end_io_data = NULL;

    rq->__sector = bio->bi_iter.bi_sector;
    rq->write_hint = bio->bi_write_hint;

    rq->__data_len = bio->bi_iter.bi_size;
    rq->bio = rq->biotail = bio;
    rq->ioprio = bio->bi_ioprio;
    rq->rq_disk = bio->bi_disk;

    printk("request_layer_km: bio_to_rq: %d\n", bio->bi_iter.bi_size);

    return rq;
}

/**
 * MY FUNCTIONS
 * */


static int __init init_request_layer_km(void)
{
    char *dev = "/dev/sdb";
    struct block_device *bdev;

    struct blk_mq_hw_ctx **hctxs, *hctx;
    int nr_hw_queues;
    struct bio *bio;
    struct request *rq;
    struct page *page;
    bool ret;

    //Allocate some kmem
    virt_mem = kmalloc(4096, GFP_KERNEL | GFP_DMA);
    memset(virt_mem, 4, 4096);
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
    bio = create_bio(bdev, &page, 1, 0, REQ_OP_WRITE | REQ_OP_DRV_IN);
    printk("request_layer_km: Create bio %p\n", bio);
    //Get HW queues associated with bdev
    hctxs = queue->queue_hw_ctx;
    printk("request_layer_km: Get hctxs %p\n", hctxs);
    nr_hw_queues = queue->nr_hw_queues;
    printk("request_layer_km: nr_hw_queues %d\n", nr_hw_queues);
    hctx = hctxs[0];
    printk("request_layer_km: hctx %p\n", hctx);
    //Create request to hctx
    rq = bio_to_rq(queue, hctx, bio, 1);
    printk("request_layer_km: bio_to_rq\n");
    //Execute request
    blk_execute_rq_nowait(rq->q, bdev->bd_disk, rq, false, rq->end_io);
    printk("request_layer_km: ret %d\n", ret);
    return 0;
}

static void __exit exit_request_layer_km(void)
{
}

module_init(init_request_layer_km)
module_exit(exit_request_layer_km)
