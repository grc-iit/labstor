//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_SHMEM_SPINLOCK_H
#define LABSTOR_SHMEM_SPINLOCK_H

#include <time.h>

namespace labstor::ipc {

struct spin_lock_header {
    uint32_t lock_;
    uint32_t max_time_ms_;
    size_t timestamp_;
};

struct SpinLock {
    spin_lock_header *header_;


    inline void Init(void *region_, uint32_t max_time_ms) {
        header_ = (spin_lock_header*)(region_);
        header_->lock_ = 0;
        header_->max_time_ms_ = 0;
        header_->timestamp_ = 0;
    }

    inline size_t get_time_ms() {
        struct timespec cur;
        size_t cur_time_ms;
        clock_gettime(CLOCK_MONOTONIC_RAW, &cur);
        cur_time_ms = cur.tv_sec*1000000000 + cur.tv_nsec;
        return cur_time_ms;
    }

    inline void Lock() {
        size_t cur_time_ms;
        uint32_t is_unlocked = 0, lock = 1;
        while(!__atomic_compare_exchange_n(&header_->lock_, &is_unlocked, lock, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
            cur_time_ms = get_time_ms();
            if(cur_time_ms - header_->timestamp_ > header_->max_time_ms_) {
                __atomic_compare_exchange_n(&header_->lock_, &lock, is_unlocked, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
            }
        }
        header_->timestamp_ = get_time_ms();
    }

    inline void UnLock() {
        header_->lock_ = 0;
    }
};

}

#endif //LABSTOR_SHMEM_SPINLOCK_H
