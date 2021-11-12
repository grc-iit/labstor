//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_PRIVATE_SHMEM_ALLOCATOR_H
#define LABSTOR_PRIVATE_SHMEM_ALLOCATOR_H

#include "allocator.h"
#include <labstor/types/data_structures/shmem_ring_buffer.h>

#ifdef __cplusplus

#include "allocator.h"
#include <stddef.h>

namespace labstor {

struct private_shmem_allocator_header {
    uint32_t region_size_;
    uint32_t request_unit_;
    shmem_ring_buffer_header objs_;
};

class private_shmem_allocator : public GenericAllocator {
private:
    void *region_;
    uint32_t region_size_;
    private_shmem_allocator_header *header_;
    shmem_ring_buffer objs_;
public:
    private_shmem_allocator() = default;

    inline void Init(void *region, uint32_t region_size, uint32_t request_unit) {
        uint32_t max_objs = 2 * region_size / request_unit, i = 0;
        void *remainder;
        uint32_t remainder_size, remainder_objs;

        region_ = region;
        region_size_ = region_size;
        header_ = (private_shmem_allocator_header*)region;
        header_->region_size_ = region_size;
        header_->request_unit_ = request_unit;
        objs_.Init(&header_->objs_, region_size - sizeof(private_shmem_allocator_header), max_objs);

        remainder = objs_.GetNextSection();
        remainder_size = (uint32_t)((size_t)region + region_size - (size_t)remainder);
        remainder_objs = remainder_size / request_unit;
        for(int i = 0; i < remainder_objs; ++i) {
            objs_.Enqueue(remainder);
            remainder = (char*)remainder + request_unit;
        }
    }

    inline void Attach(void *region) override {
        region_ = region;
        header_ = (private_shmem_allocator_header*)region;
        region_size_ = header_->region_size_;
        objs_.Attach(&header_->objs_);
    }

    inline void* Alloc(uint32_t size, uint32_t core) override {
        return objs_.Dequeue();
    }

    inline void Free(void *data) override {
        objs_.Enqueue(data);
    }
};

}

#endif

#ifdef KERNEL_BUILD

#endif

#endif //private_shmem_allocator_H
