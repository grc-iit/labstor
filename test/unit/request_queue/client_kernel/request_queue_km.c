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
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <labstor/kernel/server/types/data_structures/request_queue.h>
#include <labstor/kernel/server/module_manager.h>
#include <secure_shmem/secure_shmem.h>
#include "request_queue_test.h"
#include "kernel_server.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that reads in requests from a request queue");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("request_queue_test_km");

//Prototypes
static int __init init_request_queue_test_km(void);
static void __exit exit_request_queue_test_km(void);

inline int dequeue_requests(struct request_queue_test_request *req) {
    struct labstor_request_queue q;
    struct simple_request* rq;
    struct shmem_region_info *region = find_shmem_region(req->region_id);
    int i;

    //Attach queue to SHMEM region
    if(region == NULL) {
        pr_err("Could not find SHMEM region");
        return -1;
    }
    labstor_request_queue_attach(&q, region->vmalloc_ptr);

    //Receiving queue data
    for(i = 0; i <= 10; ++i) {
        rq = (struct simple_request*)labstor_request_queue_dequeue(&q);
        if(rq == NULL) {
            break;
        }
        pr_info("DEQUEUE IN KM(a): %lu\n", (size_t)rq);
        if(i == 10) {
            pr_err("ERROR: dequeue failed: %lu %lu\n", q.header_->enqueued_, q.header_->dequeued_);
            return -1;
        }
        pr_info("DEQUEUE IN KM(b): %d\n", rq->data);
    }
    pr_info("FINISHED DEQUEUEING!");

    //Enqueueing queue data
    for(i = 0; i < 10; ++i) {
        rq = (struct simple_request*)labstor_request_queue_allocate(&q, sizeof(struct simple_request));
        if(rq == NULL) {
            pr_warn("Could not allocate request\n");
            return -1;
        }
        rq->data = i + 100;
        pr_info("ENQUEUE IN KM: %d\n", rq->data);
        labstor_request_queue_enqueue(&q, (struct labstor_request*)rq, 1);
    }
    return 0;
}

void process_request_fn_netlink(int pid, struct request_queue_test_request *req) {
    int code = 0;
    switch(req->op) {
        case REQUEST_QUEUE_TEST_PKG_DEQUEUE: {
            code = dequeue_requests(req);
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }
    }
}

struct labstor_module request_queue_test_pkg = {
    .module_id = REQUEST_QUEUE_TEST_ID,
    .process_request_fn_netlink= (process_request_fn_netlink_type)process_request_fn_netlink,
    .get_ops = NULL
};

static int __init init_request_queue_test_km(void) {
    register_labstor_module(&request_queue_test_pkg);
    return 0;
}

static void __exit exit_request_queue_test_km(void) {
    unregister_labstor_module(&request_queue_test_pkg);
}

module_init(init_request_queue_test_km)
module_exit(exit_request_queue_test_km)
