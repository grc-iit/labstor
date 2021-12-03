//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_LABSTOR_REQUEST_MAP_H
#define LABSTOR_LABSTOR_REQUEST_MAP_H

#include <labstor/kernel/types/data_structures/shmem_qtok.h>
#include <labstor/kernel/constants/macros.h>

static const labstor_off_t null1_mark = 1 << 31;
static const labstor_off_t null1_null = ~((uint32_t)0) ^ null1_mark;

struct labstor_request_map_bucket {
    labstor_off_t off_;
};

static inline void labstor_request_map_bucket_Init(struct labstor_request_map_bucket *bucket, struct labstor_request *rq, void *region);
static inline struct labstor_request* labstor_request_map_bucket_GetValue(struct labstor_request_map_bucket *bucket, void *region);
static inline uint32_t labstor_request_map_bucket_GetKey(struct labstor_request_map_bucket *bucket, void *region);
static inline labstor_off_t labstor_request_map_bucket_GetAtomicValue(struct labstor_request_map_bucket *bucket);
static inline labstor_off_t labstor_request_map_bucket_GetAtomicKey(struct labstor_request_map_bucket *bucket);
static inline uint32_t labstor_request_map_bucket_hash(const uint32_t qtok, void *region);
static inline bool labstor_request_map_bucket_IsMarked(struct labstor_request_map_bucket *bucket);
static inline bool labstor_request_map_bucket_IsNull(struct labstor_request_map_bucket *bucket);
static inline labstor_off_t labstor_request_map_bucket_GetMarkedAtomicKey(struct labstor_request_map_bucket *bucket);
static inline labstor_off_t labstor_request_map_bucket_NullKey(void);

static inline void labstor_request_map_bucket_Init(struct labstor_request_map_bucket *bucket, struct labstor_request *rq, void *region) {
    bucket->off_ = LABSTOR_REGION_SUB(rq, region);
}

static inline struct labstor_request* labstor_request_map_bucket_GetValue(struct labstor_request_map_bucket *bucket, void *region) {
    if(labstor_request_map_bucket_IsNull(bucket)) {
        return NULL;
    }
    return (struct labstor_request *)LABSTOR_REGION_ADD(bucket->off_, region);
}

static inline uint32_t labstor_request_map_bucket_GetKey(struct labstor_request_map_bucket *bucket, void *region) {
    struct labstor_request *rq;
    if(labstor_request_map_bucket_IsNull(bucket)) { return labstor_request_map_bucket_NullKey(); }
     rq = (struct labstor_request *)LABSTOR_REGION_ADD(bucket->off_, region);
    return rq->req_id_;
}

static inline labstor_off_t labstor_request_map_bucket_GetAtomicValue(struct labstor_request_map_bucket *bucket) {
    return bucket->off_;
}

static inline labstor_off_t labstor_request_map_bucket_GetAtomicKey(struct labstor_request_map_bucket *bucket) {
    return bucket->off_;
}

static inline labstor_off_t* labstor_request_map_bucket_GetAtomicKeyRef(struct labstor_request_map_bucket *bucket) {
    return &bucket->off_;
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

static inline labstor_off_t labstor_request_map_bucket_NullKey() {
    return null1_null;
}

static inline bool labstor_request_map_bucket_KeyCompare(uint32_t req_id1, uint32_t req_id2) {
    return req_id1 == req_id2;
}

#include "shmem_unordered_map_request.h"

#endif //LABSTOR_LABSTOR_REQUEST_MAP_H
