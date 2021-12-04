//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_uint32_t_uint32_t_H
#define LABSTOR_INT_MAP_uint32_t_uint32_t_H

#include <labstor/types/data_structures/unordered_map/constants.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/types/shmem_type.h>

struct labstor_uint32_t_uint32_t_bucket {
    uint32_t key_;
    uint32_t value_;
};

static inline void labstor_uint32_t_uint32_t_bucket_Init(labstor_uint32_t_uint32_t_bucket *bucket, uint32_t key, uint32_t value) {
    bucket->key_ = key;
    bucket->value_ = value;
}
static inline uint32_t labstor_uint32_t_uint32_t_bucket_GetValue(labstor_uint32_t_uint32_t_bucket *bucket, void *region) {
    return bucket->value_;
}
static inline uint32_t labstor_uint32_t_uint32_t_bucket_GetKey(labstor_uint32_t_uint32_t_bucket *bucket, void *region) {
    return bucket->key_;
}
static inline uint32_t labstor_uint32_t_uint32_t_bucket_GetAtomicValue(labstor_uint32_t_uint32_t_bucket *bucket) {
    return bucket->value_;
}
static inline uint32_t labstor_uint32_t_uint32_t_bucket_GetAtomicKey(labstor_uint32_t_uint32_t_bucket *bucket) {
    return bucket->key_;
}
static inline uint32_t* labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef(labstor_uint32_t_uint32_t_bucket *bucket) {
    return &bucket->key_;
}
static inline uint32_t labstor_uint32_t_uint32_t_bucket_hash(const uint32_t key, const void *region) {
    return key;
}
static inline bool labstor_uint32_t_uint32_t_bucket_IsMarked(labstor_uint32_t_uint32_t_bucket *bucket) {
    return labstor_uint32_t_uint32_t_bucket_GetAtomicKey(bucket) & null1_mark;
}
static inline bool labstor_uint32_t_uint32_t_bucket_IsNull(labstor_uint32_t_uint32_t_bucket *bucket) {
    return labstor_uint32_t_uint32_t_bucket_GetAtomicKey(bucket) == null1_null;
}
static inline uint32_t labstor_uint32_t_uint32_t_bucket_GetMarkedAtomicKey(labstor_uint32_t_uint32_t_bucket *bucket) {
    return labstor_uint32_t_uint32_t_bucket_GetAtomicKey(bucket) | null1_mark;
}
static inline uint32_t labstor_uint32_t_uint32_t_bucket_NullKey() {
    return null1_null;
}
static inline bool labstor_uint32_t_uint32_t_bucket_KeyCompare(uint32_t key1, uint32_t key2) {
    return key1==key2;
}

#include <labstor/types/data_structures/unordered_map/shmem_unordered_map_uint32_t_uint32_t_impl.h>

namespace labstor::ipc {
class int_map_uint32_t_uint32_t : public unordered_map_uint32_t_uint32_t {
public:
    inline bool Set(uint32_t key, uint32_t value) {
        labstor_uint32_t_uint32_t_bucket bucket;
        labstor_uint32_t_uint32_t_bucket_Init(&bucket, key, value);
        return unordered_map_uint32_t_uint32_t::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_uint32_t_uint32_t_H
