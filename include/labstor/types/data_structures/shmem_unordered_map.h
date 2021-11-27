//
// Created by lukemartinlogan on 11/24/21.
//

#ifndef LABSTOR_SHMEM_UNORDERED_MAP_H
#define LABSTOR_SHMEM_UNORDERED_MAP_H

#include <labstor/util/errors.h>
#include "shmem_array.h"

namespace labstor::ipc {

/*
template<typename S, typename T, typename S_Atomic = S>
struct unordered_map_bucket {
    inline virtual T GetValue(void *region) = 0;
    inline virtual S GetKey(void *region) = 0;
    inline virtual S_Atomic& GetAtomicKey() = 0;
    inline virtual S_Atomic GetMarkedAtomicKey() = 0;
    inline virtual S_Atomic IsMarked() = 0;
    inline virtual S_Atomic IsNull() = 0;
    inline static S_Atomic Null();
    inline static uint32_t hash(const S &key, const void *region);
};*/

template<typename S_Atomic>
struct unordered_map_atomics_null0 {
    static const S_Atomic mark = 1 << (sizeof(S_Atomic)*8 - 1);
    static const S_Atomic null = 0;
};

template<typename S_Atomic>
struct unordered_map_atomics_null1 {
    static const S_Atomic mark = 1 << (sizeof(S_Atomic)*8 - 1);
    static const S_Atomic null = (~((S_Atomic)0)) ^ mark;
};

template<typename S, typename T, typename S_Atomic, typename bucket_t>
class unordered_map : public shmem_type {
protected:
    array<bucket_t> buckets_;
    array<bucket_t> overflow_;
public:
    static uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions) {
        return array<uint8_t>::GetSize(num_buckets) + array<bucket_t>::GetSize(max_collisions);
    }
    inline uint32_t GetSize() {
        return buckets_.GetSize() + overflow_.GetSize();
    }
    inline uint32_t GetNumBuckets() {
        return buckets_.GetLength();
    }
    inline uint32_t GetOverflow() {
        return overflow_.GetLength();
    }

    inline void Init(void *region, uint32_t region_size, uint32_t max_collisions) {
        uint32_t overflow_region_size = array<bucket_t>::GetSize(max_collisions);
        uint32_t bucket_region_size = region_size - overflow_region_size;
        buckets_.Init(region, bucket_region_size);
        region = buckets_.GetNextSection();
        overflow_.Init(region, overflow_region_size);

        //Initialize buckets
        for(int i = 0; i < buckets_.GetLength(); ++i) { buckets_[i].GetAtomicKey() = bucket_t::Null(); }
        for(int i = 0; i < overflow_.GetLength(); ++i) { overflow_[i].GetAtomicKey() = bucket_t::Null(); }
    }
    inline void Attach(void *region) {
        buckets_.Attach(region);
        region = buckets_.GetNextSection();
        overflow_.Attach(region);
    }

    inline bool Set(bucket_t &bucket) {
        uint32_t b = bucket_t::hash(bucket.GetKey(region_), region_) % buckets_.GetLength();
        if(AtomicSetKeyValue(buckets_, b, bucket)) {
            return true;
        }
        for(int i = 0; i < overflow_.GetLength(); ++i) {
            if(AtomicSetKeyValue(overflow_, i, bucket)) {
                return true;
            }
        }
        return false;
    }

    inline bool Find(S key, T &value) {
        uint32_t b = bucket_t::hash(key, region_) % buckets_.GetLength();

        //Check the primary map first
        if(AtomicGetValueByKey(buckets_, b, key, value)) { return true; }

        //Check the collisions second
        for(int i = 0; i < overflow_.GetLength(); ++i) {
            if(AtomicGetValueByKey(overflow_, i, key, value)) { return true; }
        }

        return false;
    }


    inline bool Remove(S key) {
        uint32_t b = bucket_t::hash(key, region_) % buckets_.GetLength();

        //Check the primary map first
        if(AtomicNullifyKey(buckets_, b, key)) { return true;}

        //Check the collision set second
        for(int i = 0; i < overflow_.GetLength(); ++i) {
            if(AtomicNullifyKey(overflow_, i, key)) { return true;}
        }

        return false;
    }

    inline T operator [](S key) {
        T value;
        if(Find(key, value)) {
            return value;
        }
        throw INVALID_UNORDERED_MAP_KEY.format();
    }

private:
    inline bool AtomicSetKeyValue(array<bucket_t> &arr, int i, bucket_t &bucket) {
        S_Atomic null = bucket_t::Null();
        if(__atomic_compare_exchange_n(&arr[i].GetAtomicKey(), &null, bucket.GetMarkedAtomicKey(), false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
            arr[i] = bucket;
            return true;
        }
        return false;
    }
    inline bool AtomicGetValueByKey(array<bucket_t> &arr, int i, S &key, T &value) {
        bucket_t tmp;
        do {
            tmp = arr[i];
            if (tmp.GetKey(region_) != key) { return false; }
            value = tmp.GetValue(region_);
        } while(!__atomic_compare_exchange_n(&arr[i].GetAtomicKey(), &tmp.GetAtomicKey(), tmp.GetAtomicKey(), false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
        return true;
    }
    inline bool AtomicNullifyKey(array<bucket_t> &arr, int i, S &key) {
        S_Atomic null = bucket_t::Null();
        bucket_t tmp;
        do {
            tmp = arr[i];
            if (tmp.GetKey(region_) != key) { return false; }
        } while(!__atomic_compare_exchange_n(&arr[i].GetAtomicKey(), &tmp.GetAtomicKey(), null, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
        return true;
    }
};

}

#endif //LABSTOR_SHMEM_UNORDERED_MAP_H
