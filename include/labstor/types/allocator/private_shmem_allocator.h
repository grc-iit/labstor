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

struct labstor_private_shmem_allocator_entry {
    uint32_t stamp_;
};

struct labstor_private_shmem_allocator_header {
    uint32_t region_size_;
    uint32_t request_unit_;
};

#ifdef __cplusplus
struct labstor_private_shmem_allocator : public labstor::GenericAllocator {
#else
struct labstor_private_shmem_allocator {
#endif
    void *base_region_;
    struct labstor_private_shmem_allocator_header *header_;
    struct labstor_ring_buffer_labstor_off_t objs_;

#ifdef __cplusplus
    inline void* GetRegion();
    inline void* GetBaseRegion();
    inline uint32_t GetSize();
    inline void Init(void *base_region, void *region, uint32_t region_size, uint32_t request_unit);
    inline void Attach(void *base_region, void *region);
    inline void* Alloc(uint32_t size, uint32_t core) override;
    inline void Free(void *data) override;
#endif
};

static inline void* labstor_private_shmem_allocator_GetRegion(struct labstor_private_shmem_allocator *alloc) {
    return alloc->header_;
}
static inline void* labstor_private_shmem_allocator_GetBaseRegion(struct labstor_private_shmem_allocator *alloc) {
    return alloc->base_region_;
}
static inline uint32_t labstor_private_shmem_allocator_GetSize( struct labstor_private_shmem_allocator *alloc) {
    return sizeof(struct labstor_private_shmem_allocator) + labstor_ring_buffer_labstor_off_t_GetSize(&alloc->objs_);
}

static inline void labstor_private_shmem_allocator_Init(
        struct labstor_private_shmem_allocator *alloc, void *base_region, void *region, uint32_t region_size, uint32_t request_unit) {
    uint32_t max_objs, i;
    void *remainder;
    uint32_t remainder_size, remainder_objs;

    max_objs = region_size / request_unit;
    i = 0;

    alloc->base_region_ = base_region;
    alloc->header_ = (struct labstor_private_shmem_allocator_header*)region;
    alloc->header_->region_size_ = region_size;
    alloc->header_->request_unit_ = request_unit;
    labstor_ring_buffer_labstor_off_t_Init(
            &alloc->objs_, alloc->header_+1, region_size - sizeof(struct labstor_private_shmem_allocator_header), max_objs);

    remainder = labstor_ring_buffer_labstor_off_t_GetNextSection(&alloc->objs_);
    remainder_size = (uint32_t)((size_t)region + region_size - (size_t)remainder);
    remainder_objs = remainder_size / request_unit;
    for(i = 0; i < remainder_objs; ++i) {
        labstor_ring_buffer_labstor_off_t_Enqueue_simple(&alloc->objs_, LABSTOR_REGION_SUB(remainder, alloc->base_region_));
        remainder = LABSTOR_REGION_ADD(request_unit, remainder);
    }
}

static inline void labstor_private_shmem_allocator_Attach(struct labstor_private_shmem_allocator *alloc, void *base_region, void *region) {
    alloc->base_region_ = base_region;
    alloc->header_ = (struct labstor_private_shmem_allocator_header*)region;
    labstor_ring_buffer_labstor_off_t_Attach(&alloc->objs_, alloc->header_ + 1);
}

static inline void* labstor_private_shmem_allocator_Alloc(struct labstor_private_shmem_allocator *alloc, uint32_t size, uint32_t core) {
    labstor_off_t off;
    if(!labstor_ring_buffer_labstor_off_t_Dequeue(&alloc->objs_, &off)) { return NULL; }
    return LABSTOR_REGION_ADD(off, alloc->base_region_);
}

static inline bool labstor_private_shmem_allocator_Free(struct labstor_private_shmem_allocator *alloc, void *data) {
    return labstor_ring_buffer_labstor_off_t_Enqueue_simple(&alloc->objs_, LABSTOR_REGION_SUB(data, alloc->base_region_));
}

#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_private_shmem_allocator private_shmem_allocator;
}

void* labstor_private_shmem_allocator::GetRegion() {
    return labstor_private_shmem_allocator_GetRegion(this);
}
uint32_t labstor_private_shmem_allocator::GetSize() {
    return labstor_private_shmem_allocator_GetSize(this);
}
void labstor_private_shmem_allocator::Init(void *base_region, void *region, uint32_t region_size, uint32_t request_unit) {
    labstor_private_shmem_allocator_Init(this, base_region, region, region_size, request_unit);
}
void labstor_private_shmem_allocator::Attach(void *base_region, void *region) {
    labstor_private_shmem_allocator_Attach(this, base_region, region);
}
void* labstor_private_shmem_allocator::Alloc(uint32_t size, uint32_t core) {
    return labstor_private_shmem_allocator_Alloc(this, size, core);
}
void labstor_private_shmem_allocator::Free(void *data) {
    labstor_private_shmem_allocator_Free(this, data);
}

#endif

#endif //private_shmem_allocator_H
