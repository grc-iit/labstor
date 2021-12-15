//
// Created by lukemartinlogan on 9/16/21.
//

#ifndef LABSTOR_WORKER_ORCHESTRATOR_KERNEL_H
#define LABSTOR_WORKER_ORCHESTRATOR_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>

#define WORKER_MODULE_ID "WORKER_ID"

enum {
    LABSTOR_SPAWN_WORKERS,
    LABSTOR_SET_WORKER_AFFINITY,
    LABSTOR_ASSIGN_QP,
    LABSTOR_PAUSE_WORKER,
    LABSTOR_RESUME_WORKER,
};

struct labstor_spawn_worker_request {
    struct labstor_request header;
    int num_workers;
    int region_id;
    size_t region_size;
    size_t time_slice_us;
};

struct labstor_assign_queue_pair_request {
    struct labstor_request header;
    size_t qp;
    struct labstor_queue_pair_ptr qp_ptr;
    int worker_id;
};

struct labstor_set_worker_affinity_request {
    struct labstor_request header;
    int worker_id;
    int cpu_id;
};

struct labstor_pause_worker_request {
    struct labstor_request header;
    int worker_id;
};

struct labstor_resume_worker_request {
    struct labstor_request header;
    int worker_id;
};

#endif //LABSTOR_WORKER_ORCHESTRATOR_KERNEL_H
