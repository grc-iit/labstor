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

#include <ipc_test/kernel/ipc_test_kernel.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A dummy kernel module for performance tests");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("ipc_test");

inline void complete_test(struct labstor_queue_pair *qp, struct labstor_submit_ipc_test_request *rq) {
    pr_debug("Finished assigning bdev\n");
    rq->header_.ns_id_ = 1234;
    labstor_queue_pair_Complete(qp, (struct labstor_request*)rq, (struct labstor_request*)rq);
}

void ipc_test_process_request_fn(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    switch(rq->op_) {
        case LABSTOR_START_IPC_TEST: {
            complete_test(qp, (struct labstor_submit_ipc_test_request *)rq);
            break;
        }
    }
}

struct labstor_module ipc_test_pkg = {
    .module_id = IPC_TEST_MODULE_ID,
    .runtime_id = IPC_TEST_MODULE_RUNTIME_ID,
    .process_request_fn = ipc_test_process_request_fn,
};

/**
 * MY FUNCTIONS
 * */


static int __init init_ipc_test(void) {
    register_labstor_module(&ipc_test_pkg);
    pr_info("IPCTest has started");
    return 0;
}

static void __exit exit_ipc_test(void) {
    unregister_labstor_module(&ipc_test_pkg);
    pr_info("IPCTest has ended");
}

module_init(init_ipc_test)
module_exit(exit_ipc_test)
