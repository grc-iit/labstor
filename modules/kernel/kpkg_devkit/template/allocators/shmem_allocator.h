//
// Created by lukemartinlogan on 11/7/21.
//

#ifndef LABSTOR_SMALL_SHMEM_ALLOCATOR_H
#define LABSTOR_SMALL_SHMEM_ALLOCATOR_H

#include "allocator.h"
#include "private_shmem_allocator.h"

#ifdef __cplusplus

#include <sys/sysinfo.h>
#include "allocator.h"

namespace labstor::ipc {

struct shmem_allocator_entry {
    int core_;
};

struct shmem_allocator_header {
    uint32_t region_size_;
    int concurrency_;
};

class shmem_allocator : public GenericAllocator {
private:
    int concurrency_;
    uint32_t region_size_;
    shmem_allocator_header *header_;
    std::vector<private_shmem_allocator> per_core_allocs_;
public:
    inline shmem_allocator() = default;

    inline void* GetRegion() { return header_; }

    inline uint32_t GetSize() {
        return region_size_;
    }

    inline void Init(void *region, uint32_t region_size, uint32_t request_unit, int concurrency = 0) {
        uint32_t per_core_region_size;
        void *core_region;

        if(concurrency == 0) {
            concurrency = get_nprocs_conf();
        }

        region_size_ = region_size;
        header_ = (shmem_allocator_header *)region;
        header_->region_size_ = region_size;
        header_->concurrency_ = concurrency;
        concurrency_ = concurrency;

        core_region = (void*)(header_ + 1);
        per_core_region_size = region_size / concurrency;
        per_core_allocs_.resize(concurrency);
        for(int i = 0; i < concurrency; ++i) {
            per_core_allocs_[i].Init(core_region, per_core_region_size, request_unit);
            core_region = (void*)((char*)core_region + per_core_region_size);
        }
    }

    inline void Attach(void *region) override {
        uint32_t per_core_region_size;
        void *core_region;

        header_ = (shmem_allocator_header *)region;
        region_size_ = header_->region_size_;
        concurrency_ = header_->concurrency_;

        core_region = (void*)(header_ + 1);
        per_core_region_size = region_size_ / concurrency_;
        per_core_allocs_.resize(concurrency_);
        for(int i = 0; i < concurrency_; ++i) {
            per_core_allocs_[i].Attach(core_region);
            core_region = (void*)((char*)core_region + per_core_region_size);
        }
    }

    inline void *Alloc(uint32_t size, uint32_t core) override {
        int save = core % concurrency_;
        shmem_allocator_entry *page;
        do {
            page = (shmem_allocator_entry *)per_core_allocs_[core].Alloc(size, core);
            if(page) {
                page->core_ = core;
                return (void*)(page + 1);
            }
            core = (core + 1)%concurrency_;
        } while(core != save);
        return nullptr;
    }

    inline void Free(void *data) override {
        int core = ((shmem_allocator_entry*)data - 1)->core_;
        per_core_allocs_[core].Free(data);
    }
};

}

#endif

#ifdef KERNEL_BUILD



#endif

#endif //LABSTOR_SMALL_SHMEM_ALLOCATOR_H
