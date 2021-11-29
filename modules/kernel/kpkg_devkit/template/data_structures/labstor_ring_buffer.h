//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_struct labstor_ring_buffer_H
#define LABSTOR_struct labstor_ring_buffer_H

#include <labstor/types/basics.h>

struct labstor_ring_buffer_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
};

struct labstor_ring_buffer {
    struct labstor_ring_buffer_header *rbuf->header_;
    char *queue_;
};

//#define labstor_ring_buffer(suffix, T)
static inline uint32_t labstor_ring_buffer_GetSize_global_${SUFFIX}(uint32_t max_depth, uint32_t element_size) {
    return sizeof(struct labstor_ring_buffer_header) + element_size*max_depth;
}

static inline uint32_t labstor_ring_buffer_GetSize_${SUFFIX}(struct labstor_ring_buffer *rbuf, uint32_t element_size) {
    return labstor_ring_buffer_GetSize_global(rbuf, rbuf->header_->max_depth_, element_size);
}

static inline void* labstor_ring_buffer_GetRegion_${SUFFIX}() {
    return rbuf->header_;
}

static inline uint32_t labstor_ring_buffer_GetDepth_${SUFFIX}(struct labstor_ring_buffer *rbuf) {
    return (uint32_t)(rbuf->header_->enqueued_ - rbuf->header_->dequeued_);
}

static inline uint32_t labstor_ring_buffer_GetMaxDepth_${SUFFIX}(struct labstor_ring_buffer *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline void labstor_ring_buffer_Init_${SUFFIX}(struct labstor_ring_buffer *rbuf, void *region, uint32_t region_size, uint32_t max_depth = 0) {
    uint32_t min_region_size;
    rbuf->header_ = (struct labstor_ring_buffer_header*)region;
    queue_ = (char*)(rbuf->header_ + 1);
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->dequeued_ = 0;
    if(max_depth > 0) {
        min_region_size = max_depth*sizeof(${T}) + sizeof(struct labstor_ring_buffer_header);
        if(min_region_size > region_size) {
            throw INVALID_struct labstor_ring_buffer_SIZE.format(region_size, max_depth);
        }
        rbuf->header_->max_depth_ = max_depth;
    } else {
        rbuf->header_->max_depth_ = (region_size - sizeof(struct labstor_ring_buffer_header)) / sizeof(${T});
    }
}

static inline void labstor_ring_buffer_Attach_${SUFFIX}(struct labstor_ring_buffer *rbuf, void *region) {
    rbuf->header_ = (struct labstor_ring_buffer_header*)region;
    queue_ = (char*)(rbuf->header_ + 1);
}

static inline bool labstor_ring_buffer_Enqueue_simple_${SUFFIX}(struct labstor_ring_buffer *rbuf, ${T} data) {
    uint32_t enqueued;
    return labstor_ring_buffer_Enqueue_${SUFFIX}(data, &enqueued);
}

static inline bool labstor_ring_buffer_Enqueue_${SUFFIX}(struct labstor_ring_buffer *rbuf, ${T} data, uint32_t *req_id) {
    uint64_t enqueued;
    do {
        enqueued = rbuf->header_->enqueued_;
        if(labstor_ring_buffer_GetDepth(rbuf) > rbuf->header_->max_depth_) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->enqueued_, &enqueued, enqueued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    *req_id = (uint32_t)(enqueued % (1u << 31));
    ((${T}*)queue_)[req_id % rbuf->header_->max_depth_] = data;
    return true;
}

static inline bool labstor_ring_buffer_Dequeue_${SUFFIX}(struct labstor_ring_buffer *rbuf, ${T} *data) {
    uint64_t dequeued;
    do {
        dequeued = rbuf->header_->dequeued_;
        if(rbuf->header_->enqueued_ == dequeued) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->dequeued_, &dequeued, dequeued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    *data = ((${T}*)queue_)[dequeued % rbuf->header_->max_depth_];
    return true;
}

#endif //LABSTOR_struct labstor_ring_buffer_H
