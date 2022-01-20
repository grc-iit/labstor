//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_RING_BUFFER_MPMC_H
#define LABSTOR_RING_BUFFER_MPMC_H

#include <labstor/constants/busy_wait.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/mpmc/bitmap.h>
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>

namespace labstor::ipc::mpmc {

template<typename T>
struct ring_buffer_header {
    uint32_t enqueued_, dequeued_;
    uint16_t e_lock_, d_lock_;
    uint32_t max_depth_;
    labstor_bitmap_t bitmap_[];
};

template<typename T>
struct ring_buffer : public labstor::shmem_type {
    ring_buffer_header<T> *header_;
    labstor_bitmap_t *bitmap_;
    T *queue_;

    ring_buffer() = default;
    ring_buffer(void *region, uint32_t region_size, uint32_t max_depth=0) {
        Init(region, region_size, max_depth);
    }
    ring_buffer(void *region) {
        Attach(region);
    }

    static inline uint32_t GetSize(uint32_t max_depth) {
        return sizeof(struct ring_buffer) +
               sizeof(T)*max_depth
               + labstor_bitmap_GetSize(max_depth);
    }

    inline uint32_t GetSize() {
        return GetSize(header_->max_depth_);
    }

    inline void* GetRegion() {
        return header_;
    }

    inline void* GetNextSection() {
        return (char*)header_ + GetSize();
    }

    inline uint32_t GetDepth() {
        uint32_t enqueued = header_->enqueued_;
        uint32_t dequeued = header_->dequeued_;
        if(enqueued < dequeued) { return 0; }
        return enqueued - dequeued;
    }

    inline uint32_t GetMaxDepth() {
        return (uint32_t)(header_->max_depth_);
    }

    inline bool Init(void *region, uint32_t region_size, uint32_t max_depth=0) {
        header_ = (struct ring_buffer_header<T>*)region;
        header_->enqueued_ = 0;
        header_->dequeued_ = 0;
        header_->e_lock_ = 0;
        header_->d_lock_ = 0;
        if(region_size < GetSize(max_depth)) {
            throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
        }
        if(max_depth == 0) {
            max_depth = region_size - sizeof(struct ring_buffer);
            max_depth *= LABSTOR_BITMAP_ENTRIES_PER_BLOCK;
            max_depth /= (sizeof(T)*LABSTOR_BITMAP_ENTRIES_PER_BLOCK + sizeof(labstor_bitmap_t));
        }
        if(max_depth ==0) {
            throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
        }
        header_->max_depth_ = max_depth;
        bitmap_ = header_->bitmap_;
        labstor_bitmap_Init(bitmap_, header_->max_depth_);
        queue_ = (T*)(labstor_bitmap_GetNextSection(bitmap_, header_->max_depth_));
        return true;
    }

    inline void Attach(void *region) {
        header_ = (struct ring_buffer_header<T>*)region;
        bitmap_ = header_->bitmap_;
        queue_ = (T*)(labstor_bitmap_GetNextSection(bitmap_, header_->max_depth_));
    }

    inline bool Enqueue(T data, uint32_t &req_id) {
        uint32_t enqueued, dequeued, entry;
        if(LABSTOR_INF_LOCK_TRYLOCK(&header_->e_lock_)) {
            enqueued = header_->enqueued_;
            dequeued = header_->dequeued_;
            if(enqueued - dequeued >= header_->max_depth_) {
                LABSTOR_INF_LOCK_RELEASE(&header_->e_lock_);
                return false;
            }
            entry = enqueued % header_->max_depth_;
            req_id = enqueued;
            queue_[entry] = data;
            labstor_bitmap_Set(bitmap_, entry);
            ++header_->enqueued_;
            LABSTOR_INF_LOCK_RELEASE(&header_->e_lock_);
            return true;
        }
        return false;
    }

    inline bool Enqueue(T data) {
        uint32_t enqueued;
        return Enqueue(data, enqueued);
    }

    inline bool Dequeue(T &data) {
        uint32_t enqueued, dequeued, entry;
        if(LABSTOR_INF_LOCK_TRYLOCK(&header_->d_lock_)) {
            enqueued = header_->enqueued_;
            dequeued = header_->dequeued_;
            entry = dequeued % header_->max_depth_;
            if(enqueued - dequeued == 0 || !labstor_bitmap_IsSet(bitmap_, entry)) {
                LABSTOR_INF_LOCK_RELEASE(&header_->d_lock_);
                return false;
            }
            data = queue_[entry];
            labstor_bitmap_Unset(bitmap_, entry);
            ++header_->dequeued_;
            LABSTOR_INF_LOCK_RELEASE(&header_->d_lock_);
            return true;
        }
        return false;
    }

    /*For queue plugging*/

    inline bool TryEnqueueLock() {
        return LABSTOR_INF_LOCK_TRYLOCK(&header_->e_lock_);
    }
    inline bool TryDequeueLock() {
        return LABSTOR_INF_LOCK_TRYLOCK(&header_->d_lock_);
    }
    inline void AcquireEnqueueLock() {
        LABSTOR_INF_LOCK_PREAMBLE()
        LABSTOR_INF_LOCK_ACQUIRE(&header_->e_lock_);
    }
    inline void AcquireDequeueLock() {
        LABSTOR_INF_LOCK_PREAMBLE();
        LABSTOR_INF_LOCK_ACQUIRE(&header_->d_lock_);
    }
    inline bool AcquireTimedEnqueueLock(uint32_t max_ms) {
        LABSTOR_TIMED_LOCK_PREAMBLE();
        LABSTOR_TIMED_LOCK_ACQUIRE(&header_->e_lock_, max_ms);
        return LABSTOR_LOCK_TIMED_OUT();
    }
    inline bool AcquireTimedDequeueLock(uint32_t max_ms) {
        LABSTOR_TIMED_LOCK_PREAMBLE();
        LABSTOR_TIMED_LOCK_ACQUIRE(&header_->d_lock_, max_ms);
        return LABSTOR_LOCK_TIMED_OUT();
    }
    inline void ReleaseEnqueueLock() {
        LABSTOR_SPINLOCK_RELEASE(&header_->e_lock_);
    }
    inline void ReleaseDequeueLock() {
        LABSTOR_SPINLOCK_RELEASE(&header_->d_lock_);
    }
};

}

#endif //LABSTOR_RING_BUFFER_MPMC_H
