//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_H
#define LABSTOR_INT_MAP_H

#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/types/shmem_type.h>
#include "shmem_unordered_map.h"

namespace labstor::ipc {

template<typename S, typename T>
struct int_map_bucket {
    S key_;
    T value_;

    inline int_map_bucket() = default;
    inline int_map_bucket(S key, T value) {
        key_ = key;
        value_ = value;
    }
    inline T GetValue(void *region) {
        return value_;
    }
    inline S GetKey(void *region) {
        return key_;
    }
    inline T& GetAtomicValue() {
        return value_;
    }
    inline S& GetAtomicKey() {
        return key_;
    }
    inline static uint32_t hash(const uint32_t &key, const void *region) {
        return key;
    }

    inline bool IsMarked() { return GetAtomicKey() & unordered_map_atomics_null1<S>::mark; }
    inline bool IsNull() { return GetAtomicKey() == unordered_map_atomics_null1<S>::null; }
    inline S GetMarkedAtomicKey() { return GetAtomicKey() | unordered_map_atomics_null1<S>::mark; }
    inline static S Null() { return unordered_map_atomics_null1<S>::null; }
};

template<typename S, typename T>
class int_map : public unordered_map<S, T, S, int_map_bucket<S, T>> {
public:
    inline bool Set(S key, T value) {
        int_map_bucket bucket(key, value);
        return unordered_map<S, T, S, int_map_bucket<S, T>>::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_H
