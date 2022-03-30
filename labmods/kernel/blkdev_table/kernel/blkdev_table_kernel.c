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
#include <labstor/types/data_structures/c/shmem_queue_pair.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>

#include <labmods/kernel/blkdev_table/kernel/blkdev_table_kernel.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module for storing block device pointers");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("blkdev_table");

//Data structures
struct block_device *bdevs[MAX_MOUNTED_BDEVS];
struct kmem_cache *page_cache;
EXPORT_SYMBOL(page_cache);

inline void register_bdev(struct labstor_queue_pair *qp, struct labstor_blkdev_table_register_request *kern_rq) {
    struct block_device *bdev;
    int code;
    bdev = blkdev_get_by_path(kern_rq->path_, BDEV_ACCESS_FLAGS, NULL);
    pr_info("Assigning BDEV[%d]: %s\n", kern_rq->dev_id_, kern_rq->path_);
    code = -1;
    if(bdev == NULL || IS_ERR(bdev)) {
        switch(PTR_ERR(bdev)) {
            case -EACCES: {
                code = -2;
                pr_err("BDEV %s may be read-only\n", kern_rq->path_);
                break;
            }
            case -ENOTBLK: {
                code = -3;
                pr_err("BDEV %s is not a block device\n", kern_rq->path_);
                break;
            }
            case -ENOMEM: {
                code = -4;
                pr_err("BDEV %s has ran into some memory error...\n", kern_rq->path_);
                break;
            }
            default: {
                code = -5;
                pr_err("BDEV %s ran into an error: %ld?\n", kern_rq->path_, PTR_ERR(bdev));
                break;
            }
        }
        bdev = NULL;
    }
    if(kern_rq->dev_id_ < MAX_MOUNTED_BDEVS) {
        bdevs[kern_rq->dev_id_] = bdev;
    } else {
        code = -5;
        pr_err("Dev id %d is too large\n", kern_rq->dev_id_);
    }
    pr_debug("Finished assigning bdev\n");
    kern_rq->header_.code_ = code;
    labstor_queue_pair_CompleteInf(qp, (struct labstor_request*)kern_rq);
    pr_debug("Completed request\n");
}

inline void _unregister_bdev(int bdev_id) {
    if(bdevs[bdev_id] == NULL) { return; }
    blkdev_put(bdevs[bdev_id], BDEV_ACCESS_FLAGS);
    bdevs[bdev_id] = NULL;
}

inline void unregister_bdev(struct labstor_queue_pair *qp, struct labstor_blkdev_table_unregister_request *kern_rq) {
}

struct block_device* labstor_get_bdev(int bdev_id) {
    return bdevs[bdev_id];
}
EXPORT_SYMBOL(labstor_get_bdev);

void bdev_table_process_request_fn(struct labstor_queue_pair *qp, struct labstor_request *kern_rq) {
    switch(kern_rq->op_) {
        case LABSTOR_BLKDEV_TABLE_REGISTER_BDEV: {
            register_bdev(qp, (struct labstor_blkdev_table_register_request *)kern_rq);
            break;
        }
        case LABSTOR_BLKDEV_TABLE_UNREGISTER_BDEV: {
            unregister_bdev(qp, (struct labstor_blkdev_table_unregister_request *)kern_rq);
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
    int i;
    for(i = 0; i < MAX_MOUNTED_BDEVS; ++i) {
        _unregister_bdev(i);
    }
    unregister_labstor_module(&blkdev_table_pkg);
    pr_info("Blkdev table has ended");
}

module_init(init_blkdev_table)
module_exit(exit_blkdev_table)
