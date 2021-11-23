//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_PRIVATE_SHMEM_ALLOCATOR_H
#define LABSTOR_PRIVATE_SHMEM_ALLOCATOR_H

#include "allocator.h"
#include <labstor/constants/macros.h>
#include <labstor/types/data_structures/shmem_ring_buffer.h>

#ifdef __cplusplus

#include "allocator.h"
#include <stddef.h>

namespace labstor::ipc {

struct private_shmem_allocator_header {
    uint32_t region_size_;
    uint32_t request_unit_;
};

class private_shmem_allocator : public GenericAllocator {
private:
    void *region_;
    uint32_t region_size_;
    private_shmem_allocator_header *header_;
    labstor::ipc::ring_buffer<uint32_t> objs_;
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
        objs_.Init(header_+1, region_size - sizeof(private_shmem_allocator_header), max_objs);

        remainder = objs_.GetNextSection();
        remainder_size = (uint32_t)((size_t)region + region_size - (size_t)remainder);
        remainder_objs = remainder_size / request_unit;
        for(int i = 0; i < remainder_objs; ++i) {
            objs_.Enqueue(LABSTOR_REGION_SUB(remainder, region_));
            remainder = LABSTOR_REGION_ADD(request_unit, remainder);
        }
    }

    inline void Attach(void *region) override {
        region_ = region;
        header_ = (private_shmem_allocator_header*)region;
        region_size_ = header_->region_size_;
        objs_.Attach(header_ + 1);
    }

    inline void* Alloc(uint32_t size, uint32_t core) override {
        uint32_t off;
        if(!objs_.Dequeue(off)) { return nullptr; }
        return LABSTOR_REGION_ADD(off, region_);
    }

    inline void Free(void *data) override {
        objs_.Enqueue(LABSTOR_REGION_SUB(data, region_));
    }
};

}

#endif

#ifdef KERNEL_BUILD

#endif

#endif //private_shmem_allocator_H
