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

#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_work_queue.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>
#include <workers/worker_kernel.h>
#include <secure_shmem/secure_shmem.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that manages the scheduling of labstor kernel workers");
MODULE_LICENSE("GPL");
MODULE_ALIAS("labstor_kernel_worker");

struct labstor_worker_struct {
    struct task_struct *worker_task;
    struct labstor_work_queue work_queue;
};

bool keep_running = true;
int shmem_region_id;
void *shmem_region;
struct sock *nl_sk = NULL;
struct labstor_worker_struct *workers = NULL;
typedef int (*kthread_fn)(void*);

size_t time_slice_us;

int worker_runtime(struct labstor_worker_struct *worker) {
    int work_depth, qp_depth, i, j;
    struct labstor_queue_pair_ptr ptr;
    struct labstor_queue_pair qp;
    struct labstor_request *rq;
    struct labstor_module *module;

    pr_info("Worker: %p\n", worker);
    while(keep_running) {
        work_depth = labstor_work_queue_GetDepth(&worker->work_queue);
        for(i = 0; i < work_depth; ++i) {
            if (!labstor_work_queue_Dequeue(&worker->work_queue, &ptr)) { break; }
            labstor_queue_pair_InitFromPtr(&qp, &ptr, shmem_region);
            qp_depth = labstor_queue_pair_GetDepth(&qp);
            for(j = 0; j < qp_depth; ++j) {
                if(!labstor_queue_pair_Dequeue(&qp, &rq)) { break; }
                module = get_labstor_module_by_runtime_id(rq->ns_id_);
                module->process_request_fn(&qp, rq);
            }
            labstor_queue_pair_GetPointer(&qp, &ptr, shmem_region);
            labstor_work_queue_Enqueue_simple(&worker->work_queue, ptr);
        }
    }
    return 0;
}

bool spawn_workers(int num_workers, int region_id, size_t region_size, size_t time_slice_us) {
    struct labstor_worker_struct *worker;
    void *region;
    int i;
    //Create worker array
    workers = kvmalloc(num_workers * sizeof(struct labstor_worker_struct), GFP_USER);
    if(workers == NULL) {
        pr_err("Could not allocate worker array");
        return false;
    }
    //Get shared memory
    shmem_region_id = region_id;
    shmem_region = find_shmem_region(shmem_region_id);

    //Create worker threads
    for(i = 0; i < num_workers; ++i) {
        worker = workers + i;
        labstor_work_queue_Init(&worker->work_queue, region, region_size, 0);
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
    switch(rq->header.op_) {
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
        .runtime_id = WORKER_MODULE_RUNTIME_ID,
        .process_request_fn = NULL,
        .process_request_fn_netlink = (process_request_fn_netlink_type)worker_process_request_fn_netlink,
};

static int __init init_labstor_kernel_server(void) {
    register_labstor_module(&worker_module);
    return 0;
}

static void __exit exit_labstor_kernel_server(void) {
    keep_running = false;
    if(workers) {
        kvfree(workers);
    }
    unregister_labstor_module(&worker_module);
}

module_init(init_labstor_kernel_server)
module_exit(exit_labstor_kernel_server)
