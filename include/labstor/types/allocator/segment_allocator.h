
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

#ifndef LABSTOR_SEGMENT_ALLOCATOR_H
#define LABSTOR_SEGMENT_ALLOCATOR_H


#include <labstor/constants/macros.h>
#include <labstor/types/basics.h>

struct labstor_segment_allocator {
    uint32_t offset_, max_size_;
    void *region_;
#ifdef __cplusplus
    inline void Init(void *region, uint32_t size);
    inline void Attach(void *region, uint32_t size);
    template<typename T=void>
    inline T* Alloc(uint32_t size);
#endif
};

static inline void labstor_segment_allocator_Init(struct labstor_segment_allocator *alloc, void *region, uint32_t size) {
    alloc->max_size_ = size;
    alloc->offset_ = 0;
    alloc->region_ = region;
}

static inline void labstor_segment_allocator_Attach(struct labstor_segment_allocator *alloc, void *region, uint32_t size) {
    labstor_segment_allocator_Init(alloc, region, size);
}

static inline void *labstor_segment_allocator_Alloc(struct labstor_segment_allocator *alloc, uint32_t size) {
    void *region = LABSTOR_REGION_ADD(alloc->offset_, alloc->region_);
    alloc->offset_ += size;
    return region;
}

#ifdef __cplusplus
namespace labstor {
    typedef labstor_segment_allocator segment_allocator;
}

void labstor_segment_allocator::Init(void *region, uint32_t size) {
    labstor_segment_allocator_Init(this, region, size);
}

void labstor_segment_allocator::Attach(void *region, uint32_t size) {
    labstor_segment_allocator_Attach(this, region, size);
}

template<typename T=void>
T* labstor_segment_allocator::Alloc(uint32_t size) {
    return reinterpret_cast<T*>(labstor_segment_allocator_Alloc(this, size));
}
#endif

#endif //LABSTOR_SEGMENT_ALLOCATOR_H