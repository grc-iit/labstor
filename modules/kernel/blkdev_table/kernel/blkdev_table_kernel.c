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
#include <linux/blk-mq.h>
#include <linux/list.h>
#include <linux/cpumask.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>

#include <blkdev_table/kernel/blkdev_table_kernel.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module for storing block device pointers");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("blkdev_table");

//Data structures
struct block_device *bdevs[MAX_MOUNTED_BDEVS];
struct kmem_cache *page_cache;
EXPORT_SYMBOL(page_cache);

inline void register_bdev(struct labstor_queue_pair *qp, struct labstor_submit_blkdev_table_register_request *rq) {
    struct block_device *bdev;
    struct labstor_complete_blkdev_table_register_request *rq_complete;
    int code;
    bdev = blkdev_get_by_path(rq->path_, BDEV_ACCESS_FLAGS, NULL);
    pr_info("Assigning BDEV[%d]: %s\n", rq->dev_id_, rq->path_);
    code = -1;
    if(bdev == NULL || IS_ERR(bdev)) {
        switch(PTR_ERR(bdev)) {
            case -EACCES: {
                code = -2;
                pr_err("BDEV %s may be read-only\n", rq->path_);
                break;
            }
            case -ENOTBLK: {
                code = -3;
                pr_err("BDEV %s is not a block device\n", rq->path_);
                break;
            }
            case -ENOMEM: {
                code = -4;
                pr_err("BDEV %s has ran into some memory error...\n", rq->path_);
                break;
            }
            default: {
                code = -5;
                pr_err("BDEV %s ran into an error: %ld?\n", rq->path_, PTR_ERR(bdev));
                break;
            }
        }
        bdev = NULL;
    }
    if(rq->dev_id_ < MAX_MOUNTED_BDEVS) {
        bdevs[rq->dev_id_] = bdev;
    } else {
        code = -5;
        pr_err("Dev id %d is too large\n", rq->dev_id_);
    }
    pr_debug("Finished assigning bdev\n");
    rq_complete = (struct labstor_complete_blkdev_table_register_request*)rq;
    rq_complete->header_.code_ = code;
    labstor_queue_pair_Complete(qp, (struct labstor_request*)rq, (struct labstor_request*)rq_complete);
    pr_debug("Completed request\n");
}

inline void unregister_bdev(struct labstor_queue_pair *qp, struct labstor_submit_blkdev_table_unregister_request *rq) {
}

struct block_device* labstor_get_bdev(int bdev_id) {
    return bdevs[bdev_id];
}
EXPORT_SYMBOL(labstor_get_bdev);

void bdev_table_process_request_fn(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    switch(rq->op_) {
        case LABSTOR_BLKDEV_TABLE_REGISTER_BDEV: {
            register_bdev(qp, (struct labstor_submit_blkdev_table_register_request *)rq);
            break;
        }
        case LABSTOR_BLKDEV_TABLE_UNREGISTER_BDEV: {
            unregister_bdev(qp, (struct labstor_submit_blkdev_table_unregister_request *)rq);
            break;
        }
    }
}

struct labstor_module blkdev_table_pkg = {
    .module_id = BLKDEV_TABLE_MODULE_ID,
    .runtime_id = BLKDEV_TABLE_RUNTIME_ID,
    .process_request_fn = bdev_table_process_request_fn,
};

/**
 * MY FUNCTIONS
 * */


static int __init init_blkdev_table(void) {
    register_labstor_module(&blkdev_table_pkg);
    page_cache = kmem_cache_create("labstor_pgcache", MAX_PAGES_PER_GET*sizeof(struct page*), sizeof(struct page*), SLAB_HWCACHE_ALIGN, NULL);
    pr_info("Blkdev table has started");
    return 0;
}

static void __exit exit_blkdev_table(void) {
    unregister_labstor_module(&blkdev_table_pkg);
    pr_info("Blkdev table has ended");
}

module_init(init_blkdev_table)
module_exit(exit_blkdev_table)
