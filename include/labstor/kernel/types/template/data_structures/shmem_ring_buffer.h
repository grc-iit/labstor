//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef labstor_ring_buffer_{SUFFIX}_H
#define labstor_ring_buffer_{SUFFIX}_H

#include <labstor/types/basics.h>

//#define labstor_ring_buffer_{SUFFIX}({SUFFIX}, {T})

struct labstor_ring_buffer_{SUFFIX}_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
};

struct labstor_ring_buffer_{SUFFIX} {
    struct labstor_ring_buffer_{SUFFIX}_header *header_;
    {T} *queue_;
};

static inline uint32_t labstor_ring_buffer_{SUFFIX}_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_ring_buffer_{SUFFIX}_header) + sizeof({T})*max_depth;
}

static inline uint32_t labstor_ring_buffer_{SUFFIX}_GetSize(struct labstor_ring_buffer_{SUFFIX} *rbuf) {
    return labstor_ring_buffer_{SUFFIX}_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_ring_buffer_{SUFFIX}_GetRegion(struct labstor_ring_buffer_{SUFFIX} *rbuf) {
    return rbuf->header_;
}

static inline uint32_t labstor_ring_buffer_{SUFFIX}_GetDepth(struct labstor_ring_buffer_{SUFFIX} *rbuf) {
    return (uint32_t)(rbuf->header_->enqueued_ - rbuf->header_->dequeued_);
}

static inline uint32_t labstor_ring_buffer_{SUFFIX}_GetMaxDepth(struct labstor_ring_buffer_{SUFFIX} *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_ring_buffer_{SUFFIX}_Init(struct labstor_ring_buffer_{SUFFIX} *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    uint32_t min_region_size;
    rbuf->header_ = (struct labstor_ring_buffer_{SUFFIX}_header*)region;
    rbuf->queue_ = ({T}*)(rbuf->header_ + 1);
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->dequeued_ = 0;
    if(max_depth > 0) {
        min_region_size = labstor_ring_buffer_{SUFFIX}_GetSize_global(max_depth);
        if(min_region_size > region_size) {
            //pr_warn("labstor_ring_buffer_{SUFFIX}_Init: depth of %u requires at least %u bytes", max_depth, min_region_size);
            return false;
        }
        rbuf->header_->max_depth_ = max_depth;
    } else {
        rbuf->header_->max_depth_ = (region_size - sizeof(struct labstor_ring_buffer_{SUFFIX}_header)) / sizeof({T});
    }
    return true;
}

static inline void labstor_ring_buffer_{SUFFIX}_Attach(struct labstor_ring_buffer_{SUFFIX} *rbuf, void *region) {
    rbuf->header_ = (struct labstor_ring_buffer_{SUFFIX}_header*)region;
    rbuf->queue_ = ({T}*)(rbuf->header_ + 1);
}

static inline bool labstor_ring_buffer_{SUFFIX}_Enqueue(struct labstor_ring_buffer_{SUFFIX} *rbuf, {T} data, uint32_t *req_id) {
    uint64_t enqueued;
    do {
        enqueued = rbuf->header_->enqueued_;
        if(labstor_ring_buffer_{SUFFIX}_GetDepth(rbuf) > rbuf->header_->max_depth_) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->enqueued_, &enqueued, enqueued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    *req_id = (uint32_t)(enqueued % (1u << 31));
    rbuf->queue_[*req_id % rbuf->header_->max_depth_] = data;
    return true;
}

static inline bool labstor_ring_buffer_{SUFFIX}_Enqueue_simple(struct labstor_ring_buffer_{SUFFIX} *rbuf, {T} data) {
    uint32_t enqueued;
    return labstor_ring_buffer_{SUFFIX}_Enqueue(rbuf, data, &enqueued);
}

static inline bool labstor_ring_buffer_{SUFFIX}_Dequeue(struct labstor_ring_buffer_{SUFFIX} *rbuf, {T} *data) {
    uint64_t dequeued;
    do {
        dequeued = rbuf->header_->dequeued_;
        if(rbuf->header_->enqueued_ == dequeued) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->dequeued_, &dequeued, dequeued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    *data = rbuf->queue_[dequeued % rbuf->header_->max_depth_];
    return true;
}

#endif //labstor_ring_buffer_{SUFFIX}_H
