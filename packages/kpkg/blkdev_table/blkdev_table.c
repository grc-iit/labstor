//
// Created by lukemartinlogan on 5/6/21.
//

/*
 * A kernel module that constructs bio and request objects, and submits them to the underlying drivers.
 * */

#include "request_layer.h"

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

#include "unordered_map.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("request_layer_km");


//Macros
#define BDEV_ACCESS_FLAGS FMODE_READ | FMODE_WRITE | FMODE_PREAD | FMODE_PWRITE //| FMODE_EXCL

struct unordered_map bdev_map;

void register_bdev(struct queue_pair *qp, const char *path) {
    struct block_device *bdev;
    struct labstor_id id = {0};
    strcpy(id->key, path);
    bdev = blkdev_get_by_path(dev, BDEV_ACCESS_FLAGS, NULL);
    unordered_map_add(&bdev_map, id, bdev);
}

void unregister_bdev(struct queue_pair *qp, int bdev_id) {
    unordered_map_remove_idx(bdev_id);
}

struct block_device* get_bdev(int bdev_id) {
    return unordered_map_get_idx(bdev_map, bdev_id);
}

void process_request_fn(struct queue_pair *qp, struct request_layer_request *req) {
    switch(req->op) {
        case REGISTER_BDEV: {
            break;
        }
    }
}

struct package {
    .pkg_id = REQEUST_LAYER_PKG_ID,
    .process_request_fn = process_request_fn,
    .request_size = sizeof(struct request_layer_request),
    .get_ops = NULL
} request_layer_pkg;

/**
 * MY FUNCTIONS
 * */


static int __init init_request_layer_km(void)
{
    register_package(&reqest_layer_pkg);
}

static void __exit exit_request_layer_km(void)
{
    unregister_package(&request_layer_pkg);
}

module_init(init_request_layer_km)
module_exit(exit_request_layer_km)
