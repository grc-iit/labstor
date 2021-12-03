//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_H
#define LABSTOR_INT_MAP_H

#include <labstor/types/data_structures/unordered_map/constants.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/types/shmem_type.h>

struct labstor_{S}_{T_NAME}_map_bucket {
    {S} key_;
    {T} value_;
};
inline labstor_{S}_{T_NAME}_map_bucket_Init({S} key, {T} value) {
    key_ = key;
    value_ = value;
}
inline {T} labstor_{S}_{T_NAME}_map_bucket_GetValue(void *region) {
    return value_;
}
inline {S} labstor_{S}_{T_NAME}_map_bucket_GetKey(void *region) {
    return key_;
}
inline {T} labstor_{S}_{T_NAME}_map_bucket_GetAtomicValue() {
    return value_;
}
inline {S} labstor_{S}_{T_NAME}_map_bucket_GetAtomicKey() {
    return key_;
}
inline static uint32_t labstor_{S}_{T_NAME}_map_bucket_hash(const uint32_t &key, const void *region) {
    return key;
}
inline bool labstor_{S}_{T_NAME}_map_bucket_IsMarked() { return GetAtomicKey() & null1_mark; }
inline bool labstor_{S}_{T_NAME}_map_bucket_IsNull() { return GetAtomicKey() == null1_null; }
inline {S} labstor_{S}_{T_NAME}_map_bucket_GetMarkedAtomicKey() { return GetAtomicKey() | null1_mark; }
inline static {S} labstor_{S}_{T_NAME}_map_bucket_Null() { return null1_null; }

#include <labstor/types/data_structures/unordered_map/shmem_unordered_map_{S}_{T_NAME}_impl.h>

namespace labstor::ipc {
class {S}_{T_NAME}_map : public unordered_map_{S}_{T_NAME} {
public:
    inline bool Set({S} key, {T} value) {
        {S}_{T_NAME}_map_bucket bucket;
        labstor_{S}_{T_NAME}_map_bucket_Init(&bucket, key, value);
        return {S}_{T_NAME}_map::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_H
