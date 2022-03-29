//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_H
#define LABSTOR_INT_MAP_H

#include "constants.h"
#include "shmem_unordered_map.h"
#include "labstor/userspace/util/errors.h"
#include "labstor/types/shmem_type.h"

namespace labstor::ipc::mpmc {

template<typename S, typename T>
struct int_map_bucket {
    S key_;
    T value_;
    inline void Init(S key, T value) {
        key_ = key;
        value_ = value;
    }
    inline T GetValue(void *region) {
        return value_;
    }
    inline S GetKey(void *region) {
        return key_;
    }
    static inline uint32_t KeyHash(const S key, const void *region) {
        return key;
    }
    static inline bool KeyCompare(S key1, S key2) {
        return key1==key2;
    }
};

template<typename S, typename T>
class int_map : public unordered_map<S,T,int_map_bucket<S,T>> {
public:
    inline bool Set(S key, T value) {
        int_map_bucket<S,T> bucket;
        bucket.Init(key, value);
        return unordered_map<S,T,int_map_bucket<S,T>>::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_H
