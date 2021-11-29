//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_LABSTOR_REQUEST_MAP_H
#define LABSTOR_LABSTOR_REQUEST_MAP_H

#include "labstor_undered_map.h"

static const uint32_t null1_mark = 1 << 31;
static const uint32_t null1_null = (~((uint32_t)0) ^ mark;

struct labstor_request_map_bucket {
    labstor_off_t off_;
};

static inline void labstor_request_map_bucket_Init(struct labstor_request_map_bucket *bucket, struct labstor_request *rq, void *region) {
    bucket->off_ = LABSTOR_REGION_SUB(rq, region);
}

static inline struct labstor_request* labstor_request_map_bucket_GetValue(struct labstor_request_map_bucket *bucket, void *region) {
    if(labstor_request_map_bucket_IsNull(bucket)) {
        return nullptr;
    }
    return (struct labstor_request*)LABSTOR_REGION_ADD(off_, region);
}

static inline uint32_t labstor_request_map_bucket_GetKey(struct labstor_request_map_bucket *bucket, void *region) {
    if(labstor_request_map_IsNull(bucket)) { return labstor_request_map_Null(); }
    struct labstor_request *rq = (struct labstor_request *)LABSTOR_REGION_ADD(off_, region);
    return rq->req_id_;
}

static inline labstor_off_t labstor_request_map_bucket_GetAtomicValue(struct labstor_request_map_bucket *bucket) {
    return off_;
}

static inline labstor_off_t labstor_request_map_bucket_GetAtomicKey(struct labstor_request_map_bucket *bucket) {
    return bucket->off_;
}

static inline uint32_t labstor_request_map_bucket_hash(const uint32_t qtok, void *region) {
    return qtok;
}

static inline bool labstor_request_map_bucket_IsMarked(struct labstor_request_map_bucket *bucket) {
    return labstor_request_map_bucket_GetAtomicKey(bucket) & null1_mark;
}

static inline bool labstor_request_map_bucket_IsNull(struct labstor_request_map_bucket *bucket) {
    return labstor_request_map_bucket_GetAtomicKey(bucket) == null1_null;
}

static inline labstor_off_t labstor_request_map_bucket_GetMarkedAtomicKey(struct labstor_request_map_bucket *bucket) {
    return labstor_request_map_bucket_GetAtomicKey(bucket) | null1_mark;
}

static inline static labstor_off_t labstor_request_map_bucket_Null() {
    return null1_null;
}

static inline static bool labstor_request_map_bucket_IsNullValue(struct labstor_request_map_bucket *bucket, struct labstor_request *value) {
    return value == nullptr;
}

labstor_unordered_map(
        request,
        uint32_t,
        labstor_off_t,
        labstor_off_t,
        labstor_request_map_bucket_GetAtomicKey,
        labstor_request_map_bucket_GetAtomicKeyRef,
        GetKey,
        NullKey,
        KeyCompare)

#endif //LABSTOR_LABSTOR_REQUEST_MAP_H
