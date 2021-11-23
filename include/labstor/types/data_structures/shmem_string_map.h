//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_string_map_H
#define LABSTOR_string_map_H

#include <atomic>
#include "shmem_string.h"

#ifdef __cplusplus
#include "labstor/types/shmem_type.h"

namespace labstor::ipc {

struct string_map_header {
    uint32_t num_buckets_;
};

class string_map : public shmem_type {
private:
    string_map_header *header_;
    uint32_t *buckets_;
    uint32_t num_buckets_;
public:
    static uint32_t GetSize(uint32_t num_buckets) {
        return sizeof(string_map_header) + num_buckets * sizeof(uint32_t);
    }
    inline uint32_t GetSize() {
        return GetSize(header_->num_buckets_);
    }

    void Init(void *region, uint32_t region_size) {
        uint32_t num_buckets = (region_size - sizeof(string_map_header)) / sizeof(uint32_t);
        region_ = region;
        header_ = (string_map_header * )(region_);
        num_buckets_ = num_buckets;
        header_->num_buckets_ = num_buckets;
        buckets_ = (uint32_t*)(header_ + 1);
        memset(buckets_, 0, num_buckets * sizeof(uint32_t));
    }

    void Attach(void *region) {
        region_ = region;
        header_ = (string_map_header * )(region_);
        num_buckets_ = header_->num_buckets_;
        buckets_ = (uint32_t*)(header_ + 1);
    }

    bool Set(labstor::ipc::string key, uint32_t value) {
        uint32_t b = map_key(key)%num_buckets_;
        uint32_t off = LABSTOR_REGION_SUB(key.c_str(), region_);
        for (uint32_t i = 0; i < num_buckets_; ++i) {
            if (__atomic_compare_exchange_n(&buckets_[b], &off, 0, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
                return true;
            }
            b = (b + 1) % num_buckets_;
        }
        return false;
    }

    uint32_t FindIndex(labstor::ipc::string key) {
        labstor::ipc::string bucket_key;
        uint32_t b = map_key(key)%num_buckets_;
        for (uint32_t i = 0; i < num_buckets_; ++i) {
            if(buckets_[b] == 0) { continue; }
            bucket_key.Attach(LABSTOR_REGION_ADD(buckets_[b], region_));
            if (key == bucket_key) { return b; }
            b = (b + 1) % num_buckets_;
        }
        return -1;
    }

    uint32_t *Find(labstor::ipc::string key) {
        uint32_t idx = FindIndex(key);
        if(idx == -1) { return nullptr; }
        return &buckets_[idx];
    }

    uint32_t FindIndex(std::string key) {
        labstor::ipc::string bucket_key;
        uint32_t b = map_key(key)%num_buckets_;
        for (uint32_t i = 0; i < num_buckets_; ++i) {
            if(buckets_[b] == 0) { continue; }
            bucket_key.Attach(LABSTOR_REGION_ADD(buckets_[b], region_));
            if (bucket_key == key) { return b; }
            b = (b + 1) % num_buckets_;
        }
        return -1;
    }

    uint32_t *Find(std::string key) {
        uint32_t idx = FindIndex(key);
        if(idx == -1) { return nullptr; }
        return &buckets_[idx];
    }

    void Remove(labstor::ipc::string key) {
        uint32_t idx = FindIndex(key);
        if(idx == -1) { return; }
        buckets_[idx] = 0;
    }
private:
    uint32_t map_key(labstor::ipc::string key) {
        uint32_t sum = 0;
        for(int i = 0; i < key.size(); ++i) {
            sum += key[i] << 4*(i%4);
        }
        return sum;
    }
    uint32_t map_key(std::string key) {
        uint32_t sum = 0;
        for(int i = 0; i < key.size(); ++i) {
            sum += key[i] << 4*(i%4);
        }
        return sum;
    }
};

}

#endif

#endif //LABSTOR_string_map_H
