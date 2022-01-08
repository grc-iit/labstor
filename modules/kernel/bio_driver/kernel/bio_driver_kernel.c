//
// Created by lukemartinlogan on 5/6/21.
//

/*
 * A kernel module that constructs bio and request objects, and submits them to the underlying drivers.
 * */

#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>

#include <linux/sbitmap.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/bio.h>
#include <linux/blk_types.h>
#include <linux/blkdev.h>
#include <linux/list.h>
#include <linux/cpumask.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>

#include <bio_driver/kernel/bio_driver_kernel.h>
#include <blkdev_table/kernel/blkdev_table_kernel.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("request_layer_km");

//Prototypes
static int __init init_bio_driver_km(void);
static void __exit exit_bio_driver_km(void);

/**
 * I/O REQUEST FUNCTIONS
 * */

static void labstor_complete_io(struct labstor_bio_driver_request *rq, int code) {
    rq->header_.code_ = code;
    labstor_queue_pair_CompleteInf(rq->qp_, (struct labstor_request*)rq);
}

static void io_complete(struct bio *bio) {
    int code = 0;
    struct labstor_bio_driver_request *rq;
    if(!bio || IS_ERR(bio)) {
        pr_err("io_complete was passed NULL bio");
        return;
    }
    rq = bio->bi_private;
    if(rq == NULL) {
        pr_err("io_complete request is NULL\n");
        return;
    }
    if(bio->bi_status != BLK_STS_OK) {
        code = bio->bi_status;
        pr_debug("Request did not complete: %d\n", code);
    }
    pr_debug("SUCCESS! Request: (qid=%llu,req_id=%u, code=%d)\n",
             labstor_queue_pair_GetQid(rq->qp_), rq->header_.req_id_, code);
    labstor_complete_io(rq, code);
}

static inline struct page **convert_user_buf(int pid, void *user_buf, size_t length, int *num_pagesp) {
    struct task_struct *task;
    struct page **pages;
    int num_pages;

    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    num_pages = (length % PAGE_SIZE) ? length/PAGE_SIZE + 1 : length/PAGE_SIZE;
    pages = (struct page **)kmem_cache_alloc(page_cache, GFP_KERNEL);
    if(pages == NULL) {
        pr_err("Could not allocate space for the user's pages");
        *num_pagesp = 0;
        return NULL;
    }
    get_user_pages_remote(task, task->mm, (long unsigned)user_buf, num_pages, 0, pages, NULL, NULL);

    *num_pagesp = num_pages;
    return pages;
}

static inline struct bio *create_bio(struct labstor_bio_driver_request *rq, struct block_device *bdev, struct page **pages, int num_pages, size_t sector, int op) {
    struct bio *bio;
    int i;

    bio = bio_alloc(GFP_KERNEL, num_pages);
    if(bio == NULL) {
        pr_err("Could not allocate bio request (%ld bytes)\n", sizeof(struct bio)+num_pages*sizeof(struct bio_vec));
        return NULL;
    }
    bio_set_dev(bio, bdev);
    //bio_set_op_attrs(bio, op, 0);
    bio->bi_opf = op;
    bio_set_flag(bio, BIO_USER_MAPPED);
    //bio->bi_flags |= (1U << BIO_USER_MAPPED);
    bio->bi_iter.bi_sector = sector;
    for(i = 0; i < num_pages; ++i) {
        bio_add_page(bio, pages[i], PAGE_SIZE, 0);
    }
    bio->bi_private = rq;
    bio->bi_end_io = &io_complete;
    bio->bi_status = BLK_STS_OK;
    bio->bi_ioprio = 0;
    bio->bi_write_hint = 0;
    return bio;
}

inline void submit_bio_driver_io(struct labstor_queue_pair *qp, struct labstor_bio_driver_request *rq) {
    struct page **pages;
    int success, num_pages;
    struct block_device *bdev;
    struct bio *bio;
    rq->qp_ = qp;
    success = LABSTOR_BIO_OK;

    pr_debug("Received %s request [%p]\n", (rq->header_.op_ == LABSTOR_BIO_DRIVER_WRITE) ? "REQ_OP_WRITE" : "REQ_OP_READ", rq);

    //Convert user's buffer to pages
    pr_debug("Converting user pages: %p %lu\n", rq->user_buf_, rq->buf_size_);
    pages = convert_user_buf(rq->pid_, rq->user_buf_, rq->buf_size_, &num_pages);
    if(pages == NULL) {
        pr_err("Not enough space to allocate user pages\n");
        success = LABSTOR_BIO_CANT_ALLOCATE_PAGES;
        goto err_complete_before;
    }
    //Get block device associated with semantic label
    bdev = labstor_get_bdev(rq->dev_id_);
    pr_debug("BDEV device: %p\n", bdev);
    if(bdev == NULL) {
        pr_err("Invalid block device id: %d\n", rq->dev_id_);
        success = LABSTOR_BIO_INVALID_DEVICE_ID;
        goto err_complete;
    }
    //Create bio
    bio = create_bio(rq, bdev, pages, num_pages, rq->sector_, (rq->header_.op_ == LABSTOR_BIO_DRIVER_WRITE) ? REQ_OP_WRITE : REQ_OP_READ);
    pr_debug("Create BIO: %p\n", bio);
    if(bio == NULL) {
        success = LABSTOR_BIO_CANNOT_ALLOCATE_BIO;
        pr_err("Cannot allocate more BIOs\n");
        goto err_complete;
    }
    //Submit I/O
    pr_debug("Submitting I/O\n");
    submit_bio(bio);

    //I/O was successfully submitted
    kmem_cache_free(page_cache, pages);
    return;

    //I/O was not successfully submitted (after page cache)
    err_complete:
    kmem_cache_free(page_cache, pages);

    //I/O was not successfully submitted (before page cache)
    err_complete_before:
    labstor_complete_io(rq, success);
}

inline int get_stats(struct labstor_queue_pair *qp, struct labstor_bio_driver_request *rq) {
    //int nr_hw_queues;
    //nr_hw_queues = q->nr_hw_queues;
    return 0;
}

void bio_process_request_fn(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    switch(rq->op_) {
        case LABSTOR_BIO_DRIVER_WRITE:
        case LABSTOR_BIO_DRIVER_READ: {
            submit_bio_driver_io(qp, (struct labstor_bio_driver_request*)rq);
            break;
        }
        default: {
            pr_err("Invalid bio request: %d\n", rq->op_);
            break;
        }
    }
}

struct labstor_module bio_driver_module = {
        .module_id = BIO_DRIVER_MODULE_ID,
        .runtime_id = BIO_DRIVER_RUNTIME_ID,
        .process_request_fn = bio_process_request_fn,
        .process_request_fn_netlink = NULL
};

/**
 * MY FUNCTIONS
 * */


static int __init init_bio_driver_km(void) {
    register_labstor_module(&bio_driver_module);
    pr_info("bio driver module started");
    return 0;
}

static void __exit exit_bio_driver_km(void) {
    unregister_labstor_module(&bio_driver_module);
    pr_info("bio driver module ended");
}

module_init(init_bio_driver_km)
module_exit(exit_bio_driver_km)
