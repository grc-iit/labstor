//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_PRIVATE_SHMEM_ALLOCATOR_KERNEL_H
#define LABSTOR_PRIVATE_SHMEM_ALLOCATOR_KERNEL_H

#include <labstor/userspace/constants/macros.h>
#include <labstor/types/basics.h>
#include "../data_structures/labstor_ring_buffer_off_t.h"

struct labstor_private_shmem_allocator_header {
    uint32_t region_size_;
    uint32_t request_unit_;
};

struct labstor_private_shmem_allocator {
    struct labstor_private_shmem_allocator_header *header_;
    struct labstor_ring_buffer_off_t objs_;
};

inline void* labstor_private_shmem_allocator_GetRegion(struct labstor_private_shmem_allocator *alloc) { return alloc->header_; }

uint32_t labstor_private_shmem_allocator_GetSize(struct labstor_private_shmem_allocator *alloc, struct labstor_private_shmem_allocator *alloc) {
    return sizeof(private_shmem_allocator_header) + labstor_ring_buffer_off_t_GetSize();
}

inline void labstor_private_shmem_allocator_Init(struct labstor_private_shmem_allocator *alloc, void *region, uint32_t region_size, uint32_t request_unit) {
    uint32_t max_objs = 2 * region_size / request_unit, i = 0;
    void *remainder;
    uint32_t remainder_size, remainder_objs;

    alloc->header_ = (private_shmem_allocator_header*)region;
    alloc->header_->region_size_ = region_size;
    alloc->header_->request_unit_ = request_unit;
    labstor_request_queue_Init_off_t(&alloc->objs_, alloc->header_+1, region_size - sizeof(private_shmem_allocator_header), max_objs);

    remainder = labstor_ring_buffer_off_t_GetNextSection(&alloc->objs_);
    remainder_size = (uint32_t)((size_t)region + region_size - (size_t)remainder);
    remainder_objs = remainder_size / request_unit;
    for(int i = 0; i < remainder_objs; ++i) {
        labstor_ring_buffer_off_t_Enqueue(&alloc->objs_, LABSTOR_REGION_SUB(remainder, alloc->header_));
        remainder = LABSTOR_REGION_ADD(request_unit, remainder);
    }
}

inline void labstor_private_shmem_allocator_Attach(struct labstor_private_shmem_allocator *alloc, void *region) override {
    alloc->header_ = (private_shmem_allocator_header*)region;
    labstor_ring_buffer_off_t_Attach(&alloc->objs_, alloc->header_ + 1);
}

inline void* labstor_private_shmem_allocator_Alloc(struct labstor_private_shmem_allocator *alloc, uint32_t size, uint32_t core) override {
    labstor::off_t off;
    if(!labstor_ring_buffer_off_t_Dequeue(&alloc->objs_, off)) { return nullptr; }
    return LABSTOR_REGION_ADD(off, alloc->header_);
}

inline void labstor_private_shmem_allocator_Free(struct labstor_private_shmem_allocator *alloc, void *data) override {
    labstor_ring_buffer_off_t_Enqueue(&alloc->objs_, LABSTOR_REGION_SUB(data, alloc->header_));
}

#endif //private_shmem_allocator_H
