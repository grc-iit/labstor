
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

#ifndef LABSTOR_THREAD_LOCAL_H
#define LABSTOR_THREAD_LOCAL_H

#include <labstor/types/basics.h>
#include <unistd.h>

namespace labstor {

extern uint32_t thread_local_counter_;
extern thread_local uint32_t thread_local_initialized_;
extern thread_local uint32_t thread_local_tid_;

class ThreadLocal {
public:
    static inline int GetTid() {
        if(!thread_local_initialized_) {
            thread_local_tid_ = __atomic_fetch_add(&thread_local_counter_,1,__ATOMIC_RELAXED);
            thread_local_initialized_ = true;
        }
        return thread_local_tid_;
    }
};

}

#endif //LABSTOR_THREAD_LOCAL_H