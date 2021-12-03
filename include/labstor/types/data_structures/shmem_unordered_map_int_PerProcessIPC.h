//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_H
#define LABSTOR_INT_MAP_H

#include <labstor/types/data_structures/unordered_map/constants.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/types/shmem_type.h>

struct labstor_int_PerProcessIPC_map_bucket {
    int key_;
    PerProcessIPC* value_;
};
inline labstor_int_PerProcessIPC_map_bucket_Init(int key, PerProcessIPC* value) {
    key_ = key;
    value_ = value;
}
inline PerProcessIPC* labstor_int_PerProcessIPC_map_bucket_GetValue(void *region) {
    return value_;
}
inline int labstor_int_PerProcessIPC_map_bucket_GetKey(void *region) {
    return key_;
}
inline PerProcessIPC* labstor_int_PerProcessIPC_map_bucket_GetAtomicValue() {
    return value_;
}
inline int labstor_int_PerProcessIPC_map_bucket_GetAtomicKey() {
    return key_;
}
inline static uint32_t labstor_int_PerProcessIPC_map_bucket_hash(const uint32_t &key, const void *region) {
    return key;
}
inline bool labstor_int_PerProcessIPC_map_bucket_IsMarked() { return GetAtomicKey() & null1_mark; }
inline bool labstor_int_PerProcessIPC_map_bucket_IsNull() { return GetAtomicKey() == null1_null; }
inline int labstor_int_PerProcessIPC_map_bucket_GetMarkedAtomicKey() { return GetAtomicKey() | null1_mark; }
inline static int labstor_int_PerProcessIPC_map_bucket_Null() { return null1_null; }

#include <labstor/types/data_structures/unordered_map/shmem_unordered_map_int_PerProcessIPC_impl.h>

namespace labstor::ipc {
class int_PerProcessIPC_map : public unordered_map_int_PerProcessIPC {
public:
    inline bool Set(int key, PerProcessIPC* value) {
        int_PerProcessIPC_map_bucket bucket;
        labstor_int_PerProcessIPC_map_bucket_Init(&bucket, key, value);
        return int_PerProcessIPC_map::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_H
