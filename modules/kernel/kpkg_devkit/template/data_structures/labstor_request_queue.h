//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

#include "labstor_ring_buffer.h"
#include <labstor/types/shmem_qtok.h>

#define labstor_ring_buffer(request, labstor_off_t)

struct labstor_request_queue_header {
    labstor_qid_t qid_;
    uint32_t update_lock_;
};

struct labstor_request_queue {
    struct labstor_request_queue_header *header_;
    struct labstor_ring_buffer queue_;
};

static inline uint32_t labstor_request_queue_GetSize(uint32_t max_depth) {
    return sizeof(struct labstor_request_queue_header) + labstor_ring_buffer_GetSize_request(max_depth);
}

static inline uint32_t labstor_request_queue_GetSize(struct labstor_request_queue *lrq) {
    return labstor_request_queue_GetSize(labstor_ring_buffer_GetMaxDepth_request(&lrq->queue_));
}

static inline void* labstor_request_queue_GetRegion() {
    return header_;
}

static inline uint32_t labstor_request_queue_GetDepth() {
    return queue_.GetDepth();
}
static inline uint32_t labstor_request_queue_GetFlags() {
    return header_->qid_;
}


static inline void labstor_request_queue_Init(void *region, uint32_t region_size, labstor::ipc::qid_t qid) {
    header_ = (struct labstor_request_queue_header*)region;
    header_->qid_ = qid;
    header_->update_lock_.Init();
    update_lock_ = &header_->update_lock_;
    queue_.Init(header_+1, region_size - sizeof(struct labstor_request_queue_header));
}
static inline void labstor_request_queue_Attach(void *region) {
    header_ = (struct labstor_request_queue_header*)region;
    update_lock_ = &header_->update_lock_;
    queue_.Attach(header_ + 1);
}

static inline qid_t labstor_request_queue_GetQid() {
    return header_->qid_;
}

static inline qtok_t labstor_request_queue_Enqueue(request *rq) {
    qtok_t qtok;
    qtok.qid = header_->qid_;
    while(!queue_.Enqueue(LABSTOR_REGION_SUB(rq, header_), rq->req_id_)) {}
    qtok.req_id = rq->req_id_;
    return qtok;
}
static inline bool labstor_request_queue_Dequeue(request *&rq) {
    labstor::off_t off;
    if(!queue_.Dequeue(off)) { return false; }
    rq = reinterpret_cast<request*>(LABSTOR_REGION_ADD(off, header_));
    return true;
}


#endif //LABSTOR_REQUEST_QUEUE_H
