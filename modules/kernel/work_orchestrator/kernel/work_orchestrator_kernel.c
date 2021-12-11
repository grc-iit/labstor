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
#include <linux/sched.h>
#include <linux/time.h>

#include <linux/netlink.h>
#include <linux/connector.h>

#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_work_queue.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>
#include <work_orchestrator/work_orchestrator.h>
#include <secure_shmem/kernel/secure_shmem_kernel.h>
#include <ipc_manager/kernel/ipc_manager_kernel.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that manages the scheduling of labstor kernel workers");
MODULE_LICENSE("GPL");
MODULE_ALIAS("work_orchestrator");

struct labstor_worker_struct {
    struct task_struct *worker_task;
    struct labstor_work_queue work_queue;
};

bool keep_running = true;
int shmem_region_id;
void *shmem_region;
struct sock *nl_sk = NULL;
struct labstor_worker_struct *workers = NULL;
int num_workers = 0;
typedef int (*kthread_fn)(void*);

#define MS_TO_NS(x) x * 1000000
size_t time_slice_us;

inline void complete_invalid_request(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    struct labstor_reply *rq_complete = (struct labstor_reply*)rq;
    rq_complete->code_ = LABSTOR_REQUEST_FAILED;
    if(!labstor_queue_pair_CompleteQuick(qp, (struct labstor_request*)rq, (struct labstor_request*)rq_complete)) {
        pr_err("Could not complete invalid request quickly! Giving up.\n");
    }
}

int worker_runtime(struct labstor_worker_struct *worker) {
    int work_depth, qp_depth, i, j;
    struct labstor_queue_pair_ptr ptr;
    struct labstor_queue_pair qp;
    struct labstor_request *rq;
    struct labstor_module *module;
    void *region;
    ktime_t start, end;

    //Check if IPCManager is initalized & get region
    if(!ipc_manager_IsInitialized()) {
        pr_err("IPCManager was not initalized before starting the workers!\n");
        return -1;
    }
    region = ipc_manager_GetRegion();
    pr_info("Worker %p has started\n", worker);

    //Get initial time
    start = ktime_get_ns();

    while(keep_running) {
        //Process queues
        work_depth = labstor_work_queue_GetDepth(&worker->work_queue);
        for(i = 0; i < work_depth; ++i) {
            if(!labstor_work_queue_Dequeue(&worker->work_queue, &ptr)) { break; }
            labstor_queue_pair_Attach(&qp, &ptr, region);
            qp_depth = labstor_queue_pair_GetDepth(&qp);
            for(j = 0; j < qp_depth; ++j) {
                if(!labstor_queue_pair_Dequeue(&qp, &rq)) { break; }
                module = get_labstor_module_by_runtime_id(rq->ns_id_);
                if(module == NULL) {
                    pr_warn("An invalid module was requested: %d (req_id=%u, qid=%llu, rq_off=%u)\n",
                            rq->ns_id_,
                            rq->req_id_,
                            labstor_queue_pair_GetQid(&qp),
                            LABSTOR_REGION_SUB(rq, region));
                    complete_invalid_request(&qp, rq);
                    continue;
                }
                if(!module->process_request_fn) {
                    pr_warn("A module without a kernel worker component was selected: %d %s (rq_ptr=%u)\n",
                            module->runtime_id, module->module_id.key, LABSTOR_REGION_SUB(rq, region));
                    complete_invalid_request(&qp, rq);
                    continue;
                }
                module->process_request_fn(&qp, rq);
            }
            labstor_queue_pair_GetPointer(&qp, &ptr, region);
            labstor_work_queue_Enqueue_simple(&worker->work_queue, ptr);
        }

        end = ktime_get_ns();
        if(end - start > MS_TO_NS(15)) {
            start = end;
            yield();
        }
    }
    return 0;
}

bool spawn_workers(struct labstor_spawn_worker_request *rq) {
    struct labstor_worker_struct *worker;
    void *region;
    int region_id;
    uint32_t region_size;
    uint32_t work_queue_size;
    int i;

    //Get request variables
    num_workers = rq->num_workers;
    region_id = rq->region_id;
    region_size = rq->region_size;
    work_queue_size = region_size / num_workers;

    //Create worker array
    workers = kvmalloc(num_workers * sizeof(struct labstor_worker_struct), GFP_USER);
    if(workers == NULL) {
        pr_err("Could not allocate worker array");
        return false;
    }
    //Get shared memory
    shmem_region_id = region_id;
    shmem_region = labstor_find_shmem_region(shmem_region_id);
    region = shmem_region;

    //Create worker threads
    for(i = 0; i < num_workers; ++i) {
        worker = workers + i;
        labstor_work_queue_Init(&worker->work_queue, region, work_queue_size, 0);
        region = labstor_work_queue_GetNextSection(&worker->work_queue);
        worker->worker_task = kthread_create((kthread_fn)worker_runtime, worker, "labstor_worker%d", i);
    }
    return true;
}

bool register_qp(struct labstor_assign_queue_pair_request *rq) {
    struct labstor_worker_struct *worker = &workers[rq->worker_id];
    labstor_work_queue_Enqueue_simple(&worker->work_queue, rq->ptr);
    pr_debug("Registered queue pair: %d %d\n", rq->ptr.cq_off, rq->ptr.sq_off);
    return true;
}

void pause_worker(int worker_id) {
    //set_task_state(workers[worker_id].worker_task, TASK_INTERRUPTIBLE);
}

void resume_worker(int worker_id) {
    //set_task_state(workers[worker_id].worker_task, TASK_RUNNING);
}

void set_worker_affinity(struct labstor_set_worker_affinity_request *rq) {
    pr_debug("Setting CPU to %d for worker %d\n", rq->cpu_id, rq->worker_id);
    kthread_bind(workers[rq->worker_id].worker_task, rq->cpu_id);
    wake_up_process(workers[rq->worker_id].worker_task);
}

void pause_worker_user(struct labstor_pause_worker_request *rq) {
    //set_task_state(TASK_INTERRUPTIBLE);
    //schedule();
    //https://www.linuxjournal.com/article/8144
}

void resume_worker_user(struct labstor_resume_worker_request *rq) {
    //set_task_state(TASK_RUNNING)
    //schedule();
}

void worker_process_request_fn_netlink(int pid, struct labstor_request *rq) {
    int code = 0;
    switch(rq->op_) {
        case LABSTOR_SPAWN_WORKERS: {
            pr_info("Spawning workers\n");
            code = spawn_workers((struct labstor_spawn_worker_request *)rq);
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case LABSTOR_ASSIGN_QP: {
            pr_debug("Registering queue pairs\n");
            code = register_qp((struct labstor_assign_queue_pair_request *)rq);
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case LABSTOR_SET_WORKER_AFFINITY: {
            pr_debug("Set worker affinity\n");
            set_worker_affinity((struct labstor_set_worker_affinity_request *)rq);
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case LABSTOR_PAUSE_WORKER: {
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case LABSTOR_RESUME_WORKER: {
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

static int __init init_work_orchestrator_kernel(void) {
    register_labstor_module(&worker_module);
    pr_info("Work orchestrator started");
    return 0;
}

static void __exit exit_work_orchestrator_kernel(void) {
    keep_running = false;
    /*if(workers) {
        kvfree(workers);
    }*/
    unregister_labstor_module(&worker_module);
    pr_info("Work orchestrator ended");
}

module_init(init_work_orchestrator_kernel)
module_exit(exit_work_orchestrator_kernel)
