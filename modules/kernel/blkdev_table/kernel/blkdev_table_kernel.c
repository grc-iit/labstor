//
// Created by lukemartinlogan on 5/6/21.
//

/*
 * A kernel module that constructs bio and request objects, and submits them to the underlying drivers.
 * */

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

#include <blkdev_table/blkdev_table.h>
#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("blkdev_table_km");


//Macros
#define BDEV_ACCESS_FLAGS FMODE_READ | FMODE_WRITE | FMODE_PREAD | FMODE_PWRITE //| FMODE_EXCL

#define MAX_MOUNTED_BDEVS 64
struct block_device *bdevs[MAX_MOUNTED_BDEVS];

void register_bdev(struct labstor_queue_pair *qp, const char *path, int bdev_id) {
    struct block_device *bdev;
    bdev = blkdev_get_by_path(path, BDEV_ACCESS_FLAGS, NULL);
    bdevs[bdev_id] = bdev;
}

void unregister_bdev(struct labstor_queue_pair *qp, int bdev_id) {
}

struct block_device* labstor_get_bdev(int bdev_id) {
    return bdevs[bdev_id];
}
EXPORT_SYMBOL(labstor_get_bdev);

void bdev_table_process_request_fn(struct queue_pair *qp, struct bdev_table_request *rq) {
    switch(rq->header.op) {
        case LABSTOR_BLKDEV_TABLE_REGISTER_BDEV: {
            register_bdev(qp, rq->path, rq->reg.bdev_id);
            break;
        }
        case LABSTOR_BLKDEV_TABLE_UNREGISTER_BDEV: {
            unregister_bdev(qp, rq->unreg.bdev_id)
            break;
        }
    }
}

struct module {
    .module_id = BLDEV_TABLE_MODULE_ID,
    .runtime_id = BLKDEV_TABLE_RUNTIME_ID,
    .process_request_fn = bdev_table_process_request_fn,
} blkdev_table_pkg;

/**
 * MY FUNCTIONS
 * */


static int __init init_request_layer_km(void) {
    register_labstor_module(&blkdev_table_pkg);
}

static void __exit exit_request_layer_km(void) {
    unregister_labstor_module(&blkdev_table_pkg);
}

module_init(init_request_layer_km)
module_exit(exit_request_layer_km)
