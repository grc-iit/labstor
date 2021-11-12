//
// Created by lukemartinlogan on 11/7/21.
//

#ifndef LABSTOR_SHMEM_RING_BUFFER_H
#define LABSTOR_SHMEM_RING_BUFFER_H

#include <labstor/types/basics.h>

#ifdef __cplusplus

#include <labstor/util/errors.h>

namespace labstor {

struct shmem_ring_buffer_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
    SpinLock lock_;
};

class shmem_ring_buffer {
private:
    void *region_;
    shmem_ring_buffer_header *header_;
    uint32_t *queue_;
public:
    inline uint32_t GetSize() {
        return sizeof(shmem_ring_buffer_header) + sizeof(uint32_t)*header_->max_depth_;
    }

    inline void* GetNextSection() { return (void*)((char*)region_ + GetSize()); }

    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0) {
        uint32_t min_region_size;
        region_ = region;
        header_ = (shmem_ring_buffer_header*)region;
        queue_ = (uint32_t*)(header_ + 1);
        header_->enqueued_ = 0;
        header_->dequeued_ = 0;
        header_->lock_.Init();
        if(max_depth > 0) {
            min_region_size = max_depth*sizeof(uint32_t) + sizeof(shmem_ring_buffer_header);
            if(min_region_size > region_size) {
                throw INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
            }
            header_->max_depth_ = max_depth;
        } else {
            header_->max_depth_ = (region_size - sizeof(shmem_ring_buffer_header)) / sizeof(uint32_t);
        }
    }

    inline void Attach(void *region) {
        region_ = region;
        header_ = (shmem_ring_buffer_header*)region;
        queue_ = (uint32_t*)(header_ + 1);
    }

    inline bool Enqueue(void *data) {
        uint64_t enqueued;
        do { enqueued = header_->enqueued_; }
        while(!__atomic_compare_exchange_n(&header_->enqueued_, &enqueued, enqueued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
        if((enqueued - header_->dequeued_) > header_->max_depth_) { return false; }
        queue_[enqueued % header_->max_depth_] = (uint32_t)((size_t)data - (size_t)region_);
        return true;
    }

    inline void *Dequeue() {
        uint64_t dequeued;
        do {
            dequeued = header_->dequeued_;
            if(header_->enqueued_ == dequeued) { return nullptr; }
        }
        while(!__atomic_compare_exchange_n(&header_->dequeued_, &dequeued, dequeued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
        return (void*)((char*)region_ + queue_[dequeued % header_->max_depth_]);
    }

    inline void Lock() { header_->lock_.Lock(); }
    inline void TryLock() { header_->lock_.TryLock(); }
    inline void UnLock() { header_->lock_.UnLock(); }
};

}

#endif

#ifdef KERNEL_BUILD

#endif

#endif //LABSTOR_SHMEM_RING_BUFFER_H
