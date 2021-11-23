//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_H
#define LABSTOR_INT_MAP_H

#include <labstor/util/errors.h>
#include "labstor/types/shmem_type.h"

namespace labstor::ipc {

template<typename S, typename T>
struct bucket {
    S key;
    T value;
};

struct int_map_header {
    uint32_t num_buckets_;
};

template<typename S, typename T>
class int_map : public shmem_type {
private:
    void *region_;
    int_map_header *header_;
    bucket<S, T> *buckets_;
    uint32_t num_buckets_;
public:
    inline static uint32_t GetSize(uint32_t max_depth) {
        return sizeof(ring_buffer_header) + sizeof(bucket<S, T>)*max_depth;
    }
    inline uint32_t GetSize() {
        return GetSize(header_->num_buckets_);
    }

    void Init(void *region, uint32_t region_size) {
        uint32_t num_buckets = (region_size - sizeof(int_map_header)) / sizeof(bucket<S, T>);
        region_ = region;
        header_ = (int_map_header *) (region_);
        num_buckets_ = num_buckets;
        header_->num_buckets_ = num_buckets;
        buckets_ = (bucket<S, T> *) (header_ + 1);
        memset(buckets_, 0, num_buckets * sizeof(bucket<S, T>));
    }

    void Attach(void *region) {
        region_ = region;
        header_ = (int_map_header *) (region_);
        num_buckets_ = header_->num_buckets_;
        buckets_ = (bucket<S, T> *) (header_ + 1);
    }

    bool Set(S key, T value) {
        uint32_t b = map_key(key) % num_buckets_;
        uint32_t empty = 0;
        for (uint32_t i = 0; i < num_buckets_; ++i) {
            if (__atomic_compare_exchange_n(&buckets_[b].key, &empty, key, false, __ATOMIC_RELAXED,
                                            __ATOMIC_RELAXED)) {
                buckets_[b].value = value;
                return true;
            }
            b = (b + 1) % num_buckets_;
        }
        return false;
    }

    uint32_t FindIndex(S key) {
        uint32_t b = map_key(key) % num_buckets_;
        for (uint32_t i = 0; i < num_buckets_; ++i) {
            if (buckets_[b].key == key) {
                return b;
            }
            b = (b + 1) % num_buckets_;
        }
        throw INVALID_UNORDERED_MAP_KEY.format();
    }

    T &Find(S key) {
        uint32_t idx = FindIndex(key);
        return buckets_[idx].value;
    }

    void Remove(S key) {
        uint32_t idx = FindIndex(key);
        buckets_[idx] = 0;
    }

    inline T& operator [](S key) {
        return Find(key);
    }
private:
    inline uint32_t map_key(S key) {
        return key;
    }
};

}

#endif //LABSTOR_INT_MAP_H
