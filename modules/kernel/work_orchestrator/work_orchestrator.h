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
    struct labstor_request header_;
    int num_workers_;
    int region_id_;
    size_t region_size_;
    size_t time_slice_us_;
};

struct labstor_assign_qp_request {
    struct labstor_queue_pair *qp_;
    struct labstor_queue_pair_ptr qp_ptr_;
    uint32_t worker_id_;
#ifdef __cplusplus
    labstor_assign_qp_request() = default;
    labstor_assign_qp_request(labstor::ipc::queue_pair *qp, labstor::ipc::queue_pair_ptr &ptr) {
        qp_ = qp;
        qp_ptr_ = ptr;
    }
#endif
};

struct labstor_assign_qp_request_vec {
    struct labstor_request header_;
    uint32_t count_;
    struct labstor_assign_qp_request rqs_[];
#ifdef __cplusplus
    static uint32_t GetSize(int count) {
        return sizeof(labstor_assign_qp_request_vec) + count*sizeof(labstor_assign_qp_request);
    }
#endif
};

struct labstor_set_worker_affinity_request {
    struct labstor_request header_;
    int worker_id_;
    int cpu_id_;
};

struct labstor_pause_worker_request{
    struct labstor_request header_;
    int worker_id_;
};

struct labstor_resume_worker_request{
    struct labstor_request header_;
    int worker_id_;
};


#endif //LABSTOR_WORKER_ORCHESTRATOR_KERNEL_H
