//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_INT_MAP_labstor_qid_t_qp_H
#define LABSTOR_INT_MAP_labstor_qid_t_qp_H

#include <labstor/types/data_structures/unordered_map/constants.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/types/shmem_type.h>

struct labstor_labstor_qid_t_qp_bucket {
    labstor_qid_t key_;
    labstor::ipc::queue_pair value_;
};

inline void labstor_labstor_qid_t_qp_bucket_Init(labstor_labstor_qid_t_qp_bucket *bucket, labstor_qid_t key, labstor::ipc::queue_pair value) {
    bucket->key_ = key;
    bucket->value_ = value;
}
inline labstor::ipc::queue_pair labstor_labstor_qid_t_qp_bucket_GetValue(labstor_labstor_qid_t_qp_bucket *bucket, void *region) {
    return bucket->value_;
}
inline labstor_qid_t labstor_labstor_qid_t_qp_bucket_GetKey(labstor_labstor_qid_t_qp_bucket *bucket, void *region) {
    return bucket->key_;
}
inline labstor::ipc::queue_pair labstor_labstor_qid_t_qp_bucket_GetAtomicValue(labstor_labstor_qid_t_qp_bucket *bucket) {
    return bucket->value_;
}
inline labstor_qid_t labstor_labstor_qid_t_qp_bucket_GetAtomicKey(labstor_labstor_qid_t_qp_bucket *bucket) {
    return bucket->key_;
}
inline labstor_qid_t* labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef(labstor_labstor_qid_t_qp_bucket *bucket) {
    return &bucket->key_;
}
inline static uint32_t labstor_labstor_qid_t_qp_bucket_hash(const labstor_qid_t key, const void *region) {
    return key;
}
inline bool labstor_labstor_qid_t_qp_bucket_IsMarked(labstor_labstor_qid_t_qp_bucket *bucket) {
    return labstor_labstor_qid_t_qp_bucket_GetAtomicKey(bucket) & null1_mark;
}
inline bool labstor_labstor_qid_t_qp_bucket_IsNull(labstor_labstor_qid_t_qp_bucket *bucket) {
    return labstor_labstor_qid_t_qp_bucket_GetAtomicKey(bucket) == null1_null;
}
inline labstor_qid_t labstor_labstor_qid_t_qp_bucket_GetMarkedAtomicKey(labstor_labstor_qid_t_qp_bucket *bucket) {
    return labstor_labstor_qid_t_qp_bucket_GetAtomicKey(bucket) | null1_mark;
}
inline static labstor_qid_t labstor_labstor_qid_t_qp_bucket_NullKey() {
    return null1_null;
}
inline bool labstor_labstor_qid_t_qp_bucket_KeyCompare(labstor_qid_t key1, labstor_qid_t key2) {
    return key1==key2;
}

#include <labstor/types/data_structures/unordered_map/shmem_unordered_map_labstor_qid_t_qp_impl.h>

namespace labstor::ipc {
class int_map_labstor_qid_t_qp : public unordered_map_labstor_qid_t_qp {
public:
    inline bool Set(labstor_qid_t key, labstor::ipc::queue_pair value) {
        labstor_labstor_qid_t_qp_bucket bucket;
        labstor_labstor_qid_t_qp_bucket_Init(&bucket, key, value);
        return unordered_map_labstor_qid_t_qp::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_labstor_qid_t_qp_H