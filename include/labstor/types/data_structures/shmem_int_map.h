//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_H
#define LABSTOR_INT_MAP_H

#include <labstor/util/errors.h>
#include "labstor/types/shmem_type.h"
#include "shmem_unordered_map.h"

namespace labstor::ipc {

template<typename S, typename T>
struct int_map_bucket {
    S key_;
    T value_;

    int_map_bucket() = default;
    int_map_bucket(S key, T value) {
        key_ = key;
        value_ = value;
    }
    inline T GetValue(void *region) {
        return value_;
    }
    inline S GetKey(void *region) {
        return key_;
    }
    inline S& GetAtomicKey() {
        return key_;
    }
    inline static uint32_t hash(const uint32_t &key, const void *region) {
        return key;
    }

    inline S IsMarked() { return GetAtomicKey() & unordered_map_atomics_null1<S>::mark; }
    inline S IsNull() { return GetAtomicKey() == unordered_map_atomics_null1<S>::null; }
    inline S GetMarkedAtomicKey() { return GetAtomicKey() | unordered_map_atomics_null1<S>::mark; }
    inline static S Null() { return unordered_map_atomics_null1<S>::null; }
};

template<typename S, typename T>
class int_map : public unordered_map<uint32_t, T, uint32_t, int_map_bucket<S, T>> {
public:
    inline bool Set(uint32_t key, uint32_t value) {
        int_map_bucket bucket(key, value);
        return unordered_map<uint32_t, T, uint32_t, int_map_bucket<S, T>>::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_H
