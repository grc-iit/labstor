//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_SHMEM_UNORDERED_MAP_H
#define LABSTOR_SHMEM_UNORDERED_MAP_H

#include <atomic>

namespace labstor {

template<typename S, typename T>
struct bucket {
    std::atomic<S> key;
    T value;
};

struct unordered_map_header {
    size_t num_buckets_;
};

template<typename S, typename T>
class unordered_map {
private:
    void *region_;
    unordered_map_header *header_;
    bucket<S,T> *buckets_;
    size_t num_buckets_;
public:
    static size_t size(size_t num_buckets) {
        return sizeof(unordered_map_header) + num_buckets * sizeof(bucket<S,T>);
    }
    void init(void *region, size_t region_size) {
        size_t num_buckets = (region_size - sizeof(unordered_map_header))/sizeof(bucket<S,T>);
        region_ = region;
        header_ = (unordered_map_header *)(region_);
        num_buckets_ = num_buckets;
        header_->num_buckets_ = num_buckets;
        buckets_ = (bucket<S,T> *)(header_ + 1);
        memset(buckets_, 0, num_buckets*sizeof(bucket<S,T>));
    }
    void open(void *region) {
        region_ = region;
        header_ = (unordered_map_header *)(region_);
        num_buckets_ = header_->num_buckets_;
        buckets_ = (bucket<S,T> *)(header_ + 1);
    }
    size_t map_key(S key) {
        //return std::hash(key) % num_buckets_;
        return key % num_buckets_;
    }
    void create(S key, T value) {
        S expected = 0;
        size_t b = map_key(key);
        for(size_t i = 0; i < num_buckets_; ++i) {
            if(std::atomic_compare_exchange_strong(&buckets_[b].key, &expected, key)) {
                buckets_[b].value = value;
                break;
            }
            b = (b+1)%num_buckets_;
        }
    }
    T* find(S key) {
        size_t b = map_key(key);
        for(size_t i = 0; i < num_buckets_; ++i) {
            if(buckets_[b].key == key) {
                return &buckets_[b].value;
            }
            b = (b+1)%num_buckets_;
        }
        return nullptr;
    }
    void update(S key, T value) {
        size_t b = map_key(key);
        for(size_t i = 0; i < num_buckets_; ++i) {
            if(buckets_[b].key == key) {
                buckets_[b].value = value;
            }
            b = (b+1)%num_buckets_;
        }
    }
    void remove(S key) {
        size_t b = map_key(key);
        for(size_t i = 0; i < num_buckets_; ++i) {
            if(buckets_[b].key == key) {
                buckets_[b].key = 0;
            }
            b = (b+1)%num_buckets_;
        }
    }
};

}

#endif //LABSTOR_SHMEM_UNORDERED_MAP_H
