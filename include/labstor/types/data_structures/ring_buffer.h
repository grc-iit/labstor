//
// Created by lukemartinlogan on 11/7/21.
//

#ifndef LABSTOR_RING_BUFFER_H
#define LABSTOR_RING_BUFFER_H

#include <labstor/types/basics.h>

#ifdef __cplusplus

#include <labstor/util/errors.h>

namespace labstor {

struct ring_buffer_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
};

template<typename T>
class ring_buffer {
private:
    void *region_;
    ring_buffer_header *header_;
    T *queue_;
public:
    inline uint32_t GetSize() {
        return sizeof(ring_buffer_header) + sizeof(T)*header_->max_depth_;
    }

    inline void* GetNextSection() { return (void*)((char*)region_ + GetSize()); }

    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0) {
        uint32_t min_region_size;
        region_ = region;
        header_ = (ring_buffer_header*)region;
        queue_ = (T*)(header_ + 1);
        header_->enqueued_ = 0;
        header_->dequeued_ = 0;
        if(max_depth > 0) {
            min_region_size = max_depth*sizeof(uint32_t) + sizeof(ring_buffer_header);
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
        do { enqueued = header_->enqueued_; }
        while(!__atomic_compare_exchange_n(&header_->enqueued_, &enqueued, enqueued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
        if((enqueued - header_->dequeued_) > header_->max_depth_) { return false; }
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
};

}

#endif

#ifdef KERNEL_BUILD

#include <linux/types.h>

#endif

#endif //LABSTOR_RING_BUFFER_H
