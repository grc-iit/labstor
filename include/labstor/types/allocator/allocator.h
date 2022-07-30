
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

#ifndef LABSTOR_GENERIC_ALLOCATOR_H
#define LABSTOR_GENERIC_ALLOCATOR_H

#ifdef __cplusplus

#include <labstor/types/shmem_type.h>
#include <stdint.h>
#include <sched.h>
#include <labstor/types/thread_local.h>

namespace labstor {

class GenericAllocator : public shmem_type {
public:
    void* Alloc(uint32_t size) {
        return Alloc(size, labstor::ThreadLocal::GetTid());
    }
    virtual void* Alloc(uint32_t size, uint32_t core) = 0;
    virtual void Free(void *data) = 0;
};

}

#endif

#endif //LABSTOR_GENERIC_ALLOCATOR_H