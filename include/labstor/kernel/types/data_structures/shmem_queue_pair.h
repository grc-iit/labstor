
#ifndef LABSTOR_QUEUE_PAIR_KERNEL_H
#define LABSTOR_QUEUE_PAIR_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/kernel/types/data_structures/shmem_qtok.h>
#include <labstor/kernel/types/data_structures/shmem_request_queue.h>
#include <labstor/kernel/types/data_structures/shmem_request_map.h>

struct labstor_queue_pair_ptr {
    labstor_off_t sq_off;
    labstor_off_t cq_off;
    uint32_t pid;
};

struct labstor_queue_pair {
    struct labstor_request_queue sq;
    struct labstor_unordered_map_request cq;
};

static inline void labstor_queue_pair_ptr_Init(struct labstor_queue_pair_ptr *ptr, labstor_qid_t qid, void *sq_region, void *cq_region, void *region) {
    ptr->sq_off = LABSTOR_REGION_SUB(sq_region, region);
    ptr->cq_off = LABSTOR_REGION_SUB(cq_region, region);
    //ptr->pid = LABSTOR_GET_QP_PID(qid);
}

static inline void labstor_queue_pair_InitFromPtr(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *region) {
    labstor_request_queue_Attach(&qp->sq, LABSTOR_REGION_ADD(ptr->sq_off, region));
    labstor_unordered_map_request_Attach(&qp->cq, LABSTOR_REGION_ADD(ptr->cq_off, region));
}

static inline void labstor_queue_pair_GetPointer(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *region) {
    labstor_queue_pair_ptr_Init(
            ptr,
            labstor_request_queue_GetQid(&qp->sq),
            labstor_request_queue_GetRegion(&qp->sq),
            labstor_unordered_map_request_GetRegion(&qp->cq),
            region);
}

static inline void labstor_queue_pair_Init(struct labstor_queue_pair *qp, labstor_qid_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_request_queue_Init(&qp->sq, sq_region, sq_size, qid);
    labstor_unordered_map_request_Init(&qp->cq, cq_region, cq_size, 4);
}

static inline void labstor_queue_pair_Attach(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *base) {
    labstor_request_queue_Attach(&qp->sq, LABSTOR_REGION_ADD(ptr->sq_off, base));
    labstor_unordered_map_request_Attach(&qp->cq, LABSTOR_REGION_ADD(ptr->cq_off, base));
}

static inline struct labstor_qtok_t labstor_queue_pair_Enqueue(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    return labstor_request_queue_Enqueue(&qp->sq, rq);
}

static inline bool labstor_queue_pair_Dequeue(struct labstor_queue_pair *qp, struct labstor_request** rq) {
    return labstor_request_queue_Dequeue(&qp->sq, rq);
}

static inline void labstor_queue_pair_Complete(struct labstor_queue_pair *qp, struct labstor_request *rq, struct labstor_request *msg) {
    struct labstor_request_map_bucket b;
    msg->req_id_ = rq->req_id_;
    labstor_request_map_bucket_Init(&b, rq, labstor_unordered_map_request_GetRegion(&qp->cq));
    labstor_unordered_map_request_Set(&qp->cq, &b);
}

static inline struct labstor_request* labstor_queue_pair_Wait(struct labstor_queue_pair *qp, uint32_t req_id) {
    struct labstor_request *ret = NULL;
    while(!labstor_unordered_map_request_Find(&qp->cq, req_id, &ret)) {}
    labstor_unordered_map_request_Remove(&qp->cq, req_id);
    return ret;
}

static inline uint32_t labstor_queue_pair_GetDepth(struct labstor_queue_pair *qp) {
    return labstor_request_queue_GetDepth(&qp->sq);
}

#endif