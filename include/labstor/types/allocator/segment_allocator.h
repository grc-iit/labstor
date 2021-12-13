//
// Created by lukemartinlogan on 12/9/21.
//

#ifndef LABSTOR_SEGMENT_ALLOCATOR_H
#define LABSTOR_SEGMENT_ALLOCATOR_H

//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_PRIVATE_SHMEM_ALLOCATOR_KERNEL_H
#define LABSTOR_PRIVATE_SHMEM_ALLOCATOR_KERNEL_H

#include <labstor/constants/macros.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_labstor_off_t.h>
#ifdef __cplusplus
#include "allocator.h"
#endif
#include <labstor/constants/debug.h>

struct labstor_segment_allocator {
    uint32_t offset_;
    void *region_;
};

void labstor_segment_allocator_Init(struct labstor_segment_allocator *alloc, void *region) {
    alloc->offset_ = 0;
    alloc->region_ = region;
}

void *labstor_segment_allocator_Alloc(struct labstor_segment_allocator *alloc, void size) {
    void *region = LABSTOR_REGION_ADD(offset, alloc->region_);
    alloc->offset_ += size;
    return region;
}

#endif //LABSTOR_SEGMENT_ALLOCATOR_H
