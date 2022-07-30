
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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