//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_PRIVATE_SHMEM_ALLOCATOR_KERNEL_H
#define LABSTOR_PRIVATE_SHMEM_ALLOCATOR_KERNEL_H

#include <labstor/constants/macros.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_labstor_off_t.h>

struct labstor_private_shmem_allocator_header {
    uint32_t region_size_;
    uint32_t request_unit_;
};

struct labstor_private_shmem_allocator {
    struct labstor_private_shmem_allocator_header *header_;
    struct labstor_ring_buffer_labstor_off_t objs_;
};

static inline void* labstor_private_shmem_allocator_GetRegion(struct labstor_private_shmem_allocator *alloc) { return alloc->header_; }

static inline uint32_t labstor_private_shmem_allocator_GetSize( struct labstor_private_shmem_allocator *alloc) {
    return sizeof(struct labstor_private_shmem_allocator) + labstor_ring_buffer_labstor_off_t_GetSize(&alloc->objs_);
}

static inline void labstor_private_shmem_allocator_Init(struct labstor_private_shmem_allocator *alloc, void *region, uint32_t region_size, uint32_t request_unit) {
    uint32_t max_objs = 2 * region_size / request_unit, i = 0;
    void *remainder;
    uint32_t remainder_size, remainder_objs;

    alloc->header_ = (struct labstor_private_shmem_allocator_header*)region;
    alloc->header_->region_size_ = region_size;
    alloc->header_->request_unit_ = request_unit;
    labstor_ring_buffer_labstor_off_t_Init(&alloc->objs_, alloc->header_+1, region_size - sizeof(struct labstor_private_shmem_allocator_header), max_objs);

    remainder = labstor_ring_buffer_labstor_off_t_GetNextSection(&alloc->objs_);
    remainder_size = (uint32_t)((size_t)region + region_size - (size_t)remainder);
    remainder_objs = remainder_size / request_unit;
    for(int i = 0; i < remainder_objs; ++i) {
        labstor_ring_buffer_labstor_off_t_Enqueue_simple(&alloc->objs_, LABSTOR_REGION_SUB(remainder, alloc->header_));
        remainder = LABSTOR_REGION_ADD(request_unit, remainder);
    }
}

static inline void labstor_private_shmem_allocator_Attach(struct labstor_private_shmem_allocator *alloc, void *region) {
    alloc->header_ = (struct labstor_private_shmem_allocator_header*)region;
    labstor_ring_buffer_labstor_off_t_Attach(&alloc->objs_, alloc->header_ + 1);
}

static inline void* labstor_private_shmem_allocator_Alloc(struct labstor_private_shmem_allocator *alloc, uint32_t size, uint32_t core) {
    labstor_off_t off;
    if(!labstor_ring_buffer_labstor_off_t_Dequeue(&alloc->objs_, &off)) { return nullptr; }
    return LABSTOR_REGION_ADD(off, alloc->header_);
}

static inline void labstor_private_shmem_allocator_Free(struct labstor_private_shmem_allocator *alloc, void *data) {
    labstor_ring_buffer_labstor_off_t_Enqueue_simple(&alloc->objs_, LABSTOR_REGION_SUB(data, alloc->header_));
}

#ifdef __cplusplus

#include "allocator.h"

namespace labstor::ipc {

class private_shmem_allocator : private labstor_private_shmem_allocator, public GenericAllocator {
public:
    private_shmem_allocator() = default;
    inline void* GetRegion() {
        return labstor_private_shmem_allocator_GetRegion(this);
    }
    uint32_t GetSize() {
        return labstor_private_shmem_allocator_GetSize(this);
    }

    inline void Init(void *region, uint32_t region_size, uint32_t request_unit) {
        labstor_private_shmem_allocator_Init(this, region, region_size, request_unit);
    }

    inline void Attach(void *region) override {
        labstor_private_shmem_allocator_Attach(this, region);
    }

    inline void* Alloc(uint32_t size, uint32_t core) override {
        return labstor_private_shmem_allocator_Alloc(this, size, core);
    }

    inline void Free(void *data) override {
        labstor_private_shmem_allocator_Free(this, data);
    }
};

}

#endif

#endif //private_shmem_allocator_H
