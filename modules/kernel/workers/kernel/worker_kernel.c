//
// Created by lukemartinlogan on 5/6/21.
//

#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/kthread.h>

#include <linux/netlink.h>
#include <linux/connector.h>

#include <kpkg_devkit/request_queue.h>
#include <kpkg_devkit/module_registrar.h>
#include <workers/worker_kernel.h>
#include "kernel_server.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that manages the scheduling of labstor kernel workers");
MODULE_LICENSE("GPL");
MODULE_ALIAS("labstor_kernel_worker");

struct labstor_worker_struct {
    struct task_struct *worker_task;
    //struct labstor_request_queue worker_queue;
};

size_t time_slice_us_;
struct sock *nl_sk = NULL;
struct labstor_worker_struct *workers = NULL;
typedef int (*kthread_fn)(void*);

size_t time_slice_us;

int worker_runtime(struct labstor_worker_struct *worker) {
    pr_info("Worker: %p\n", worker);
    while(true) {
        //labstor_request_queue_dequeue(&worker->worker_queue);
    }
    return 0;
}

bool spawn_workers(int num_workers, int region_id, size_t region_size, size_t time_slice_us) {
    struct labstor_worker_struct *worker;
    void *region;
    int i;
    time_slice_us_ = time_slice_us;
    workers = vmalloc(num_workers * sizeof(struct labstor_worker_struct));
    if(workers == NULL) {
        pr_err("Could not allocate worker array");
        return false;
    }
    //region = shmem whatever...
    for(i = 0; i < num_workers; ++i) {
        worker = workers + i;
        //labstor_request_queue_init(&worker->worker_queue, region, region_size, time_slice_us);
        worker->worker_task = kthread_run((kthread_fn)worker_runtime, worker, "labstor_worker%d", i);
    }
    return true;
}

void set_worker_affinity(int worker_id, int cpu_id) {
    kthread_bind(workers[worker_id].worker_task, cpu_id);
}

void pause_worker(int worker_id) {
}

void resume_worker(int worker_id) {
}


void worker_process_request_fn_netlink(int pid, struct kernel_worker_request *rq) {
    int code = 0;
    switch(rq->op) {
        case SPAWN_WORKERS: {
            code = spawn_workers(rq->spawn.num_workers, rq->spawn.region_id, rq->spawn.region_size, rq->spawn.time_slice_us);
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case SET_WORKER_AFFINITY: {
            set_worker_affinity(rq->affinity.worker_id, rq->affinity.cpu_id);
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case PAUSE_WORKER: {
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case RESUME_WORKER: {
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }
    }
}

struct labstor_module worker_module = {
        .module_id = WORKER_MODULE_ID,
        .process_request_fn = NULL,
        .process_request_fn_netlink = (process_request_fn_netlink_type)worker_process_request_fn_netlink,
        .get_ops = NULL
};

static int __init init_labstor_kernel_server(void) {
    register_labstor_module(&worker_module);
    return 0;
}

static void __exit exit_labstor_kernel_server(void) {
    if(workers) {
        vfree(workers);
    }
    unregister_labstor_module(&worker_module);
}

module_init(init_labstor_kernel_server)
module_exit(exit_labstor_kernel_server)
