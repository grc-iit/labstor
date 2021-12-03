//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_H
#define LABSTOR_INT_MAP_H

#include <labstor/types/data_structures/unordered_map/constants.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/types/shmem_type.h>

struct labstor_qid_t_qp_map_bucket {
    qid_t key_;
    labstor::ipc::queue_pair value_;
};
inline labstor_qid_t_qp_map_bucket_Init(qid_t key, labstor::ipc::queue_pair value) {
    key_ = key;
    value_ = value;
}
inline labstor::ipc::queue_pair labstor_qid_t_qp_map_bucket_GetValue(void *region) {
    return value_;
}
inline qid_t labstor_qid_t_qp_map_bucket_GetKey(void *region) {
    return key_;
}
inline labstor::ipc::queue_pair labstor_qid_t_qp_map_bucket_GetAtomicValue() {
    return value_;
}
inline qid_t labstor_qid_t_qp_map_bucket_GetAtomicKey() {
    return key_;
}
inline static uint32_t labstor_qid_t_qp_map_bucket_hash(const uint32_t &key, const void *region) {
    return key;
}
inline bool labstor_qid_t_qp_map_bucket_IsMarked() { return GetAtomicKey() & null1_mark; }
inline bool labstor_qid_t_qp_map_bucket_IsNull() { return GetAtomicKey() == null1_null; }
inline qid_t labstor_qid_t_qp_map_bucket_GetMarkedAtomicKey() { return GetAtomicKey() | null1_mark; }
inline static qid_t labstor_qid_t_qp_map_bucket_Null() { return null1_null; }

#include <labstor/types/data_structures/unordered_map/shmem_unordered_map_qid_t_qp_impl.h>

namespace labstor::ipc {
class qid_t_qp_map : public unordered_map_qid_t_qp {
public:
    inline bool Set(qid_t key, labstor::ipc::queue_pair value) {
        qid_t_qp_map_bucket bucket;
        labstor_qid_t_qp_map_bucket_Init(&bucket, key, value);
        return qid_t_qp_map::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_H
