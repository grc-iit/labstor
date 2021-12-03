//
// Created by lukemartinlogan on 11/7/21.
//

#ifndef LABSTOR_SMALL_SHMEM_ALLOCATOR_KERNEL_H
#define LABSTOR_SMALL_SHMEM_ALLOCATOR_KERNEL_H

#include "private_shmem_allocator.h"

struct labstor_shmem_allocator_entry {
    int core_;
};

struct labstor_shmem_allocator_header {
    uint32_t region_size_;
    int concurrency_;
};

struct labstor_shmem_allocator {
    int concurrency_;
    uint32_t region_size_;
    shmem_allocator_header *alloc->header_;
    struct private_shmem_allocator *per_core_allocs_;
};


inline void* labstor_shmem_allocator_GetRegion(struct labstor_shmem_allocator *alloc) { return alloc->header_; }

inline uint32_t labstor_shmem_allocator_GetSize(struct labstor_shmem_allocator *alloc) {
    return region_size_;
}

inline void labstor_shmem_allocator_Init(struct labstor_shmem_allocator *alloc, void *region, uint32_t region_size, uint32_t request_unit, int concurrency = 0) {
    uint32_t per_core_region_size;
    void *core_region;

    if(concurrency_ == 0) {
        concurrency_ = get_nprocs_conf();
    }

    region_size_ = region_size;
    alloc->header_ = (shmem_allocator_header *)region;
    alloc->header_->region_size_ = region_size;
    alloc->header_->concurrency_ = concurrency;
    alloc->concurrency_ = concurrency;

    core_region = (void*)(alloc->header_ + 1);
    per_core_region_size = region_size / concurrency;
#ifdef KERNEL_BUILD
    per_core_allocs_ = kmalloc(alloc->concurrency_ * sizeof(struct labstor_private_shmem_allocator));
#elif __cplusplus
    per_core_allocs_ = malloc(alloc->concurrency_ * sizeof(struct labstor_private_shmem_allocator));
#endif
    for(int i = 0; i < concurrency_; ++i) {
        labstor_private_shmem_allocator_Init(&per_core_allocs_[i], core_region, per_core_region_size, request_unit);
        core_region = (void*)((char*)core_region + per_core_region_size);
    }
}

inline void labstor_shmem_allocator_Attach(struct labstor_shmem_allocator *alloc, void *region) {
    uint32_t per_core_region_size;
    void *core_region;

    alloc->header_ = (shmem_allocator_header *)region;
    region_size_ = alloc->header_->region_size_;
    alloc->concurrency_ = alloc->header_->concurrency_;

    core_region = (void*)(alloc->header_ + 1);
    per_core_region_size = region_size_ / alloc->concurrency_;
#ifdef KERNEL_BUILD
    per_core_allocs_ = kmalloc(alloc->concurrency_ * sizeof(struct labstor_private_alloc));
#elif __cplusplus
    per_core_allocs_ = malloc(alloc->concurrency_ * sizeof(struct labstor_private_alloc));
#endif
    for(int i = 0; i < alloc->concurrency_; ++i) {
        labstor_private_shmem_allocator_Attach(&per_core_allocs_[i], core_region);
        core_region = (void*)((char*)core_region + per_core_region_size);
    }
}

inline void *labstor_shmem_allocator_Alloc(struct labstor_shmem_allocator *alloc, uint32_t size, uint32_t core) {
    int save = core % alloc->concurrency_;
    shmem_allocator_entry *page;
    do {
        page = (struct shmem_allocator_entry *)labstor_private_shmem_allocator_Alloc(&per_core_allocs_[core], size, core);
        if(page) {
            page->core_ = core;
            return (void*)(page + 1);
        }
        core = (core + 1)%alloc->concurrency_;
    } while(core != save);
    return nullptr;
}

inline void labstor_shmem_allocator_Free(struct labstor_shmem_allocator *alloc, void *data) override {
    int core = ((struct shmem_allocator_entry*)data - 1)->core_;
    labstor_private_shmem_allocator_Free(&per_core_allocs_[core], data);
}


#ifdef __cplusplus

#include <sys/sysinfo.h>
#include "allocator.h"

namespace labstor::ipc {

class shmem_allocator : public labstor_shmem_allocator, public GenericAllocator {
public:
    inline shmem_allocator() = default;
    inline void* GetRegion() {
        labstor_shmem_allocator_GetRegion(this);
    }
    inline uint32_t GetSize() {
        return labstor_shmem_allocator_GetSize(this);
    }
    inline void Init(void *region, uint32_t region_size, uint32_t request_unit, int concurrency = 0) {
        labstor_shmem_allocator_GetRegion(this, region, region_size, request_unit, concurrency);
    }
    inline void Attach(void *region) override {
        labstor_shmem_allocator_Attach(this, region);
    }
    inline void *Alloc(uint32_t size, uint32_t core) override {
        return labstor_shmem_allocator_Alloc(this, size, core);
    }
    inline void Free(void *data) override {
        return labstor_shmem_allocator_Free(this, data);
    }
};

}

#endif

#endif //LABSTOR_SMALL_SHMEM_ALLOCATOR_H
