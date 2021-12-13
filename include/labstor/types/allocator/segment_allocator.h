//
// Created by lukemartinlogan on 12/9/21.
//

#ifndef LABSTOR_SEGMENT_ALLOCATOR_H
#define LABSTOR_SEGMENT_ALLOCATOR_H

//
// Created by lukemartinlogan on 9/19/21.
//

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
    alloc->offset_ += size;
    return LABSTOR_REGION_ADD(alloc->offset_, alloc->region_);
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
