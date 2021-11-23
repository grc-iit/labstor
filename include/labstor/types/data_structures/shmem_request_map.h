//
// Created by lukemartinlogan on 11/20/21.
//

#ifndef LABSTOR_SHMEM_REQEST_MAP_H
#define LABSTOR_SHMEM_REQEST_MAP_H

#include "labstor/types/shmem_type.h"
#include <labstor/constants/macros.h>
#include <labstor/types/data_structures/shmem_request.h>

namespace labstor::ipc {

struct request_map_header {
    uint32_t num_buckets_;
};

class request_map : public shmem_type {
private:
    request_map_header *header_;
    uint32_t *buckets_;
    uint32_t num_buckets_;
public:
    static uint32_t GetSize(uint32_t num_buckets) {
        return sizeof(request_map_header) + num_buckets * sizeof(uint32_t);
    }
    inline uint32_t GetSize() {
        return GetSize(header_->num_buckets_);
    }

    void Init(void *region, uint32_t region_size) {
        uint32_t num_buckets = (region_size - sizeof(request_map_header)) / sizeof(uint32_t);
        region_ = region;
        header_ = (request_map_header*)(region_);
        num_buckets_ = num_buckets;
        header_->num_buckets_ = num_buckets;
        buckets_ = (uint32_t*)(header_ + 1);
        memset(buckets_, 0, num_buckets * sizeof(uint32_t));
    }

    void Attach(void *region) {
        region_ = region;
        header_ = (request_map_header * )(region_);
        num_buckets_ = header_->num_buckets_;
        buckets_ = (uint32_t*)(header_ + 1);
    }

    bool Set(labstor::ipc::request *rq) {
        uint32_t b = map_key(rq->qtok_);
        uint32_t off = LABSTOR_REGION_SUB(rq, region_);
        for (uint32_t i = 0; i < num_buckets_; ++i) {
            if (__atomic_compare_exchange_n(&buckets_[b], &off, 0, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
                return true;
            }
            b = (b + 1) % num_buckets_;
        }
        return false;
    }

    uint32_t FindIndex(uint32_t qtok) {
        uint32_t b = map_key(qtok);
        for (uint32_t i = 0; i < num_buckets_; ++i) {
            labstor::ipc::request *rq = (labstor::ipc::request*)LABSTOR_REGION_ADD(buckets_[b], region_);
            if (rq->qtok_ == qtok) {
                return b;
            }
            b = (b + 1) % num_buckets_;
        }
        return -1;
    }

    uint32_t *Find(uint32_t qtok) {
        uint32_t idx = FindIndex(qtok);
        if(idx == -1) { return nullptr; }
        return &buckets_[idx];
    }

    void Remove(uint32_t qtok) {
        uint32_t idx = FindIndex(qtok);
        if(idx == -1) { return; }
        buckets_[idx] = 0;
    }

private:
    uint32_t map_key(uint32_t qtok) {
        return qtok % num_buckets_;
    }
};

}
#endif //LABSTOR_SHMEM_REQEST_MAP_H
