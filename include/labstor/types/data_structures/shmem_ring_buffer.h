//
// Created by lukemartinlogan on 11/7/21.
//

#ifndef LABSTOR_SHMEM_RING_BUFFER_H
#define LABSTOR_SHMEM_RING_BUFFER_H

#include <labstor/types/basics.h>

#ifdef __cplusplus

#include <labstor/util/errors.h>
#include "labstor/types/shmem_type.h"

namespace labstor::ipc {

struct ring_buffer_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
};

template<typename T>
class ring_buffer : public shmem_type {
private:
    ring_buffer_header *header_;
    T *queue_;
public:
    inline static uint32_t GetSize(uint32_t max_depth) {
        return sizeof(ring_buffer_header) + sizeof(T)*max_depth;
    }
    inline uint32_t GetSize() {
        return GetSize(header_->max_depth_);
    }

    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0) {
        uint32_t min_region_size;
        region_ = region;
        header_ = (ring_buffer_header*)region;
        queue_ = (T*)(header_ + 1);
        header_->enqueued_ = 0;
        header_->dequeued_ = 0;
        if(max_depth > 0) {
            min_region_size = max_depth*sizeof(T) + sizeof(ring_buffer_header);
            if(min_region_size > region_size) {
                throw INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
            }
            header_->max_depth_ = max_depth;
        } else {
            header_->max_depth_ = (region_size - sizeof(ring_buffer_header)) / sizeof(T);
        }
    }

    inline void Attach(void *region) {
        region_ = region;
        header_ = (ring_buffer_header*)region;
        queue_ = (T*)(header_ + 1);
    }

    inline bool Enqueue(T data) {
        uint64_t enqueued;
        do {
            enqueued = header_->enqueued_;
            if((enqueued - header_->dequeued_) > header_->max_depth_) { return false; }
        }
        while(!__atomic_compare_exchange_n(&header_->enqueued_, &enqueued, enqueued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
        queue_[enqueued % header_->max_depth_] = data;
        return true;
    }

    inline bool Dequeue(T &data) {
        uint64_t dequeued;
        do {
            dequeued = header_->dequeued_;
            if(header_->enqueued_ == dequeued) { return false; }
        }
        while(!__atomic_compare_exchange_n(&header_->dequeued_, &dequeued, dequeued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
        data = queue_[dequeued % header_->max_depth_];
        return true;
    }

    inline uint32_t GetDepth() {
        return (uint32_t)(header_->enqueued_ - header_->dequeued_);
    }
    inline uint32_t GetMaxDepth() {
        return (uint32_t)(header_->max_depth_);
    }
};

}

#endif

#ifdef KERNEL_BUILD

#include <linux/types.h>

#endif

#endif //LABSTOR_SHMEM_RING_BUFFER_H
