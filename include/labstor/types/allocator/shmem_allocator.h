//
// Created by lukemartinlogan on 11/7/21.
//

#ifndef LABSTOR_SMALL_SHMEM_ALLOCATOR_H
#define LABSTOR_SMALL_SHMEM_ALLOCATOR_H

#include "private_shmem_allocator.h"

#ifdef __cplusplus

#include "allocator.h"

namespace labstor {

struct shmem_allocator_entry {
    int core_;
};

struct shmem_allocator_header {
    int concurrency_;
};

class shmem_allocator : public GenericAllocator {
private:
    int concurrency_;
    void *region_;
    size_t region_size_, per_core_region_size_;
    shmem_allocator_header *header_;
    std::vector<private_shmem_allocator> per_core_allocs_;
public:
    small_shmem_allocator() = default;

    void Init(void *region, uint32_t region_size, uint32_t request_unit, int concurrency) {
        concurrency_ = concurrency;
        region_ = region;
        region_size_ = region_size;
        per_core_region_size_ = (region_size - sizeof(shmem_allocator_header)) / concurrency;
        header_ = (shmem_alocator_header*)region_;
        header_->concurrency_ = concurrency;

        uint32_t off = sizeof(shmem_allocator_header);
        per_core_allocs_.reserve(concurrency);
        for(int i = 0; i < concurrency; ++i) {
            per_core_allocs_.emplace_back();
            per_core_allocs_[i].Init((char*)region_ + off, per_core_region_size_, request_unit);
            off += per_core_region_size;
        }
    }

    void Attach(void *region, size_t region_size) {
        header_ =  (shmem_alocator_header*)region_;
        concurrency_ = header_->concurrency_;

        uint32_t off = sizeof(shmem_allocator_header);
        per_core_allocs_.reserve(concurrency);
        for(int i = 0; i < concurrency; ++i) {
            per_core_allocs_.emplace_back();
            per_core_allocs_[i].Attach((char*)region_ + off, per_core_region_size_, request_unit);
            off += per_core_region_size;
        }
    }

    void *Alloc(uint32_t size, uint32_t core) {
        private_shmem_allocator *alloc_;
        for(int i = 0; i < 2*concurrency_; ++i) {
            alloc_ = &per_core_allocs_[core];
            if(!alloc_->TryLock()) {
                core = (core + 1) % concurency_;
                continue;
            }
            shmem_allocator_entry *entry = (shmem_allocator_entry *)alloc_->Alloc(size + sizeof(shmem_allocator_entry));
            if (entry == nullptr) {
                alloc_->UnLock();
                continue;
            }
            entry->core_ = core;
            alloc_->UnLock();
            return (void*)(entry + 1);
        }
    }

    void Free(void *data) {
        int core;
        shmem_allocator_entry *entry = (shmem_allocator_entry *)data - 1;
        core = entry->core_;
        while(1) {
            alloc_ = &per_core_allocs_[core];
            if(!alloc_->TryLock()) {
                core = (core + 1) % concurency_;
                continue;
            }
            alloc_->Free((void*)entry);
        }
    }
};

}

#endif

#ifdef KERNEL_BUILD



#endif

#endif //LABSTOR_SMALL_SHMEM_ALLOCATOR_H
