//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_{S}_{T_NAME}_H
#define LABSTOR_INT_MAP_{S}_{T_NAME}_H

#include <labstor/types/data_structures/unordered_map/constants.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/types/shmem_type.h>

struct labstor_{S}_{T_NAME}_bucket {
    {S} key_;
    {T} value_;
};

inline void labstor_{S}_{T_NAME}_bucket_Init(labstor_{S}_{T_NAME}_bucket *bucket, {S} key, {T} value) {
    bucket->key_ = key;
    bucket->value_ = value;
}
inline {T} labstor_{S}_{T_NAME}_bucket_GetValue(labstor_{S}_{T_NAME}_bucket *bucket, void *region) {
    return bucket->value_;
}
inline {S} labstor_{S}_{T_NAME}_bucket_GetKey(labstor_{S}_{T_NAME}_bucket *bucket, void *region) {
    return bucket->key_;
}
inline {T} labstor_{S}_{T_NAME}_bucket_GetAtomicValue(labstor_{S}_{T_NAME}_bucket *bucket) {
    return bucket->value_;
}
inline {S} labstor_{S}_{T_NAME}_bucket_GetAtomicKey(labstor_{S}_{T_NAME}_bucket *bucket) {
    return bucket->key_;
}
inline {S}* labstor_{S}_{T_NAME}_bucket_GetAtomicKeyRef(labstor_{S}_{T_NAME}_bucket *bucket) {
    return &bucket->key_;
}
inline static uint32_t labstor_{S}_{T_NAME}_bucket_hash(const {S} key, const void *region) {
    return key;
}
inline bool labstor_{S}_{T_NAME}_bucket_IsMarked(labstor_{S}_{T_NAME}_bucket *bucket) {
    return labstor_{S}_{T_NAME}_bucket_GetAtomicKey(bucket) & null1_mark;
}
inline bool labstor_{S}_{T_NAME}_bucket_IsNull(labstor_{S}_{T_NAME}_bucket *bucket) {
    return labstor_{S}_{T_NAME}_bucket_GetAtomicKey(bucket) == null1_null;
}
inline {S} labstor_{S}_{T_NAME}_bucket_GetMarkedAtomicKey(labstor_{S}_{T_NAME}_bucket *bucket) {
    return labstor_{S}_{T_NAME}_bucket_GetAtomicKey(bucket) | null1_mark;
}
inline static {S} labstor_{S}_{T_NAME}_bucket_NullKey() {
    return null1_null;
}
inline bool labstor_{S}_{T_NAME}_bucket_KeyCompare({S} key1, {S} key2) {
    return key1==key2;
}

#include <labstor/types/data_structures/unordered_map/shmem_unordered_map_{S}_{T_NAME}_impl.h>

namespace labstor::ipc {
class int_map_{S}_{T_NAME} : public unordered_map_{S}_{T_NAME} {
public:
    inline bool Set({S} key, {T} value) {
        labstor_{S}_{T_NAME}_bucket bucket;
        labstor_{S}_{T_NAME}_bucket_Init(&bucket, key, value);
        return unordered_map_{S}_{T_NAME}::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_{S}_{T_NAME}_H
