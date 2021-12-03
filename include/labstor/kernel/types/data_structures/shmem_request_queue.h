//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

#include <labstor/kernel/constants/macros.h>
#include <labstor/kernel/types/data_structures/shmem_ring_buffer_off_t.h>
#include <labstor/kernel/types/data_structures/shmem_qtok.h>
#include <labstor/kernel/types/data_structures/shmem_request.h>

struct labstor_request_queue_header {
    labstor_qid_t qid_;
    uint32_t update_lock_;
};

struct labstor_request_queue {
    struct labstor_request_queue_header *header_;
    struct labstor_ring_buffer_off_t queue_;
};

static inline uint32_t labstor_request_queue_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_request_queue_header) + labstor_ring_buffer_off_t_GetSize_global(max_depth);
}

static inline uint32_t labstor_request_queue_GetSize(struct labstor_request_queue *lrq) {
    return labstor_request_queue_GetSize_global(labstor_ring_buffer_off_t_GetMaxDepth(&lrq->queue_));
}

static inline void* labstor_request_queue_GetRegion(struct labstor_request_queue *lrq) {
    return lrq->header_;
}

static inline uint32_t labstor_request_queue_GetDepth(struct labstor_request_queue *lrq) {
    return labstor_ring_buffer_off_t_GetDepth(&lrq->queue_);
}
static inline uint32_t labstor_request_queue_GetFlags(struct labstor_request_queue *lrq) {
    return lrq->header_->qid_;
}

static inline void labstor_request_queue_Init(struct labstor_request_queue *lrq, void *region, uint32_t region_size, labstor_qid_t qid) {
    lrq->header_ = (struct labstor_request_queue_header*)region;
    lrq->header_->qid_ = qid;
    //lrq->update_lock_ = &lrq->header_->update_lock_;
    labstor_ring_buffer_off_t_Init(&lrq->queue_, lrq->header_+1, region_size - sizeof(struct labstor_request_queue_header), 0);
}
static inline void labstor_request_queue_Attach(struct labstor_request_queue *lrq, void *region) {
    lrq->header_ = (struct labstor_request_queue_header*)region;
    //lrq->update_lock_ = &lrq->header_->update_lock_;
    labstor_ring_buffer_off_t_Attach(&lrq->queue_, lrq->header_ + 1);
}

static inline labstor_qid_t labstor_request_queue_GetQid(struct labstor_request_queue *lrq) {
    return lrq->header_->qid_;
}

static inline struct labstor_qtok_t labstor_request_queue_Enqueue(struct labstor_request_queue *lrq, struct labstor_request *rq) {
    struct labstor_qtok_t qtok;
    qtok.qid = lrq->header_->qid_;
    while(!labstor_ring_buffer_off_t_Enqueue(&lrq->queue_, LABSTOR_REGION_SUB(rq, lrq->header_), &rq->req_id_)) {}
    qtok.req_id = rq->req_id_;
    return qtok;
}

static inline bool labstor_request_queue_Dequeue(struct labstor_request_queue *lrq, struct labstor_request **rq) {
    labstor_off_t off;
    if(labstor_ring_buffer_off_t_Dequeue(&lrq->queue_, &off)) { return false; }
    *rq = (struct labstor_request*)(LABSTOR_REGION_ADD(off, lrq->header_));
    return true;
}


#endif //LABSTOR_REQUEST_QUEUE_H
