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

#include <kpkg_devkit/types.h>
#include <kpkg_devkit/requeust_queue.h>
#include "request_queue_test.h"
#include "secure_shmem.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("request_layer_km");

//Prototypes
static int __init init_request_layer_km(void);
static void __exit exit_request_layer_km(void);

inline int dequeue_requests(struct request_queue_test_request *req) {
    struct labstor_request_queue q;
    struct simple_request* rq;
    struct shmem_region_info *region = find_shmem_region(req->region_id);

    //Attach queue to SHMEM region
    if(region == NULL) {
        pr_err("Could not find SHMEM region");
        return -1;
    }
    labstor_request_queue_attach(&q, region->vmalloc_ptr);

    //Receiving queue data
    while(rq = labstor_request_queue_dequeue(&q)) {
        pr_info("DEQUEUE IN KM: %d\n", rq->data);
    }

    //Enqueueing queue data
    for(int i = 0; i < 10; ++i) {
        rq = labstor_request_queue_allocate(&q, sizeof(struct simple_request));
        if(rq == NULL) {
            pr_warn("Could not allocate request\n");
            return -1;
        }
        rq->data = i + 100;
        labstor_request_queue_enqueue(&q, rq, 1);
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

struct module {
    .module_id = REQUEST_QUEUE_TEST_ID,
    .process_request_fn_netlink= process_request_fn_netlink,
    .request_size = sizeof(struct request_queue_test_request),
    .get_ops = NULL
} request_queue_test_pkg;

static int __init init_request_layer_km(void)
{
    register_module(&request_queue_test_pkg);
}

static void __exit exit_request_layer_km(void)
{
    unregister_module(&request_queue_test_pkg);
}

module_init(init_request_layer_km)
module_exit(exit_request_layer_km)
