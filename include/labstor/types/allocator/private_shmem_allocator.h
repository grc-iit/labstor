//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_PRIVATE_SHMEM_ALLOCATOR_H
#define LABSTOR_PRIVATE_SHMEM_ALLOCATOR_H

#include <labstor/data_structures/shmem_ring_buffer.h>

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
    shmem_ring_buffer *objs_;
public:
    private_shmem_allocator() = default;
    private_shmem_allocator(void *region, uint32_t region_size, uint32_t request_unit) {
        Init(region, region_size, request_unit);
    }

    inline void Init(void *region, uint32_t region_size, uint32_t request_unit) {
        uint32_t num_objs = (region_size - sizeof(private_shmem_allocator_header)) / request_unit;

        region_ = region;
        region_size_ = region_size;
        header_ = (private_shmem_allocator_header*)region;
        header_->region_size_ = region_size;
        header_->request_unit_ = request_unit;

        objs_->Init((void*)&header_->objs_, region_size, num_objs);
        for(allocator_size_t i = sizeof(header_); i < region_size; i += request_unit) {
            objs_->Append(entry);
        }
    }

    inline void Attach(void *region, size_t region_size) {
        region_ = region;
        region_size_ = region_size;
        header_ = (private_shmem_allocator_header*)region;
    }

    inline void* Alloc(size_t size, uint32_t core) {
        shmem_linked_list_entry *entry = header_->objs_.Pop(region_);
        if(entry == nullptr) {
            return nullptr;
        }
        return (void*)(entry + 1);
    }

    inline void Free(void *data) {
        shmem_linked_list_entry *entry = (shmem_linked_list_entry*)data - 1;
        header_->objs_.Append(entry, region_);
    }
};

}

#endif

#ifdef KERNEL_BUILD

#endif

#endif //private_shmem_allocator_H
