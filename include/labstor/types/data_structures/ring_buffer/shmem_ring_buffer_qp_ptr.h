//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef labstor_ring_buffer_qp_ptr_H
#define labstor_ring_buffer_qp_ptr_H

#include <labstor/types/basics.h>

//#define labstor_ring_buffer_qp_ptr(qp_ptr, struct labstor_queue_pair_ptr)

struct labstor_ring_buffer_qp_ptr_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
};

struct labstor_ring_buffer_qp_ptr {
    struct labstor_ring_buffer_qp_ptr_header *header_;
    struct labstor_queue_pair_ptr *queue_;
};

static inline uint32_t labstor_ring_buffer_qp_ptr_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_ring_buffer_qp_ptr_header) + sizeof(struct labstor_queue_pair_ptr)*max_depth;
}

static inline uint32_t labstor_ring_buffer_qp_ptr_GetSize(struct labstor_ring_buffer_qp_ptr *rbuf) {
    return labstor_ring_buffer_qp_ptr_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_ring_buffer_qp_ptr_GetRegion(struct labstor_ring_buffer_qp_ptr *rbuf) {
    return rbuf->header_;
}

static inline uint32_t labstor_ring_buffer_qp_ptr_GetDepth(struct labstor_ring_buffer_qp_ptr *rbuf) {
    return (uint32_t)(rbuf->header_->enqueued_ - rbuf->header_->dequeued_);
}

static inline uint32_t labstor_ring_buffer_qp_ptr_GetMaxDepth(struct labstor_ring_buffer_qp_ptr *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_ring_buffer_qp_ptr_Init(struct labstor_ring_buffer_qp_ptr *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    uint32_t min_region_size;
    rbuf->header_ = (struct labstor_ring_buffer_qp_ptr_header*)region;
    rbuf->queue_ = (struct labstor_queue_pair_ptr*)(rbuf->header_ + 1);
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->dequeued_ = 0;
    if(max_depth > 0) {
        min_region_size = labstor_ring_buffer_qp_ptr_GetSize_global(max_depth);
        if(min_region_size > region_size) {
            //pr_warn("labstor_ring_buffer_qp_ptr_Init: depth of %u requires at least %u bytes", max_depth, min_region_size);
            return false;
        }
        rbuf->header_->max_depth_ = max_depth;
    } else {
        rbuf->header_->max_depth_ = (region_size - sizeof(struct labstor_ring_buffer_qp_ptr_header)) / sizeof(struct labstor_queue_pair_ptr);
    }
    return true;
}

static inline void labstor_ring_buffer_qp_ptr_Attach(struct labstor_ring_buffer_qp_ptr *rbuf, void *region) {
    rbuf->header_ = (struct labstor_ring_buffer_qp_ptr_header*)region;
    rbuf->queue_ = (struct labstor_queue_pair_ptr*)(rbuf->header_ + 1);
}

static inline bool labstor_ring_buffer_qp_ptr_Enqueue(struct labstor_ring_buffer_qp_ptr *rbuf, struct labstor_queue_pair_ptr data, uint32_t *req_id) {
    uint64_t enqueued;
    do {
        enqueued = rbuf->header_->enqueued_;
        if(labstor_ring_buffer_qp_ptr_GetDepth(rbuf) > rbuf->header_->max_depth_) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->enqueued_, &enqueued, enqueued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    *req_id = (uint32_t)(enqueued % (1u << 31));
    rbuf->queue_[*req_id % rbuf->header_->max_depth_] = data;
    return true;
}

static inline bool labstor_ring_buffer_qp_ptr_Enqueue_simple(struct labstor_ring_buffer_qp_ptr *rbuf, struct labstor_queue_pair_ptr data) {
    uint32_t enqueued;
    return labstor_ring_buffer_qp_ptr_Enqueue(rbuf, data, &enqueued);
}

static inline bool labstor_ring_buffer_qp_ptr_Dequeue(struct labstor_ring_buffer_qp_ptr *rbuf, struct labstor_queue_pair_ptr *data) {
    uint64_t dequeued;
    do {
        dequeued = rbuf->header_->dequeued_;
        if(rbuf->header_->enqueued_ == dequeued) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->dequeued_, &dequeued, dequeued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    *data = rbuf->queue_[dequeued % rbuf->header_->max_depth_];
    return true;
}

#endif //labstor_ring_buffer_qp_ptr_H
