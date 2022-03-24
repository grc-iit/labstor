//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_LABSTOR_REQUEST_MAP_H
#define LABSTOR_LABSTOR_REQUEST_MAP_H

/*
 * Assumes that one thread sets requests and another finds/removes requests
 * All entries are initially 0xFFFFFFFF (INVALID)
 * If a bucket is equal to this, it can be set from one thread.
 * A consumer thread will see that this value is not INVALID, copy, and then set the entry to INVALID again
 * */

#include "labstor/types/data_structures/shmem_qtok.h"
#include "labstor/constants/macros.h"

#define LABSTOR_INVALID_REQUEST_ID 0xFFFFFFFF

struct labstor_request_map_header {
    uint32_t num_buckets_;
};

#ifdef __cplusplus
struct labstor_request_map : public labstor::shmem_type {
#else
struct labstor_request_map {
#endif
    struct labstor_request_map_header *header_;
    labstor_off_t *buckets_;
    void *base_region_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t num_buckets);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void* GetBaseRegion();
    inline uint32_t GetNumBuckets();
    inline void Init(void *base_region, void *region, uint32_t region_size, uint32_t num_buckets);
    inline void Init(void *base_region, void *region, uint32_t region_size);
    inline void Attach(void *base_region, void *region);
    inline int Set(struct labstor_request *rq);
    inline int FindAndRemove(uint32_t key, struct labstor_request* &value);
#endif
};


static inline uint32_t labstor_request_map_GetSize_global(uint32_t num_buckets) {
    return sizeof(struct labstor_request_map_header) + sizeof(labstor_off_t)*num_buckets;
}

static inline uint32_t labstor_request_map_GetSize(struct labstor_request_map *map) {
    return labstor_request_map_GetSize_global(map->header_->num_buckets_);
}

static inline void* labstor_request_map_GetRegion(struct labstor_request_map *map) {
    return (void*)map->header_;
}

static inline void* labstor_request_map_GetBaseRegion(struct labstor_request_map *map) {
    return map->base_region_;
}

static inline uint32_t labstor_request_map_GetNumBuckets(struct labstor_request_map *map) {
    return map->header_->num_buckets_;
}

static inline bool labstor_request_map_Init(
        struct labstor_request_map *map,
        void *base_region, void *region, uint32_t region_size, uint32_t num_buckets) {
    map->base_region_ = base_region;
    map->header_ = (struct labstor_request_map_header*)region;
    if(region_size < labstor_request_map_GetSize_global(num_buckets)) {
#ifdef __cplusplus
        throw labstor::INVALID_UNORDERED_MAP_SIZE.format(region_size, num_buckets);
#else
        return false;
#endif
    }
    if(num_buckets == 0) {
        num_buckets = region_size - sizeof(struct labstor_request_map_header);
        num_buckets /= sizeof(labstor_off_t);
    }
    if(num_buckets == 0) {
#ifdef __cplusplus
        throw labstor::INVALID_UNORDERED_MAP_SIZE.format(region_size, num_buckets);
#else
        return false;
#endif
    }
    map->header_->num_buckets_ = num_buckets;
    map->buckets_ = (labstor_off_t*)(map->header_ + 1);
    memset(map->buckets_, LABSTOR_INVALID_REQUEST_ID, map->header_->num_buckets_*sizeof(labstor_off_t));
    return true;
}

static inline void labstor_request_map_Attach(
        struct labstor_request_map *map, void *base_region, void *region) {
    map->base_region_ = base_region;
    map->header_ = (struct labstor_request_map_header*)region;
    map->buckets_ = (labstor_off_t*)(map->header_ + 1);
}

static inline void labstor_request_map_RemoteAttach(
        struct labstor_request_map *map, void *kern_base_region, void *kern_region) {
    map->base_region_ = kern_base_region;
    map->header_ = (struct labstor_request_map_header*)kern_region;
    map->buckets_ = (labstor_off_t*)(map->header_ + 1);
}

static inline bool labstor_request_map_Set(struct labstor_request_map *map, struct labstor_request *rq) {
    uint32_t b = rq->req_id_ % map->header_->num_buckets_;
    if(map->buckets_[b] != -1) {
        return false;
    }
    map->buckets_[b] = LABSTOR_REGION_SUB(rq, map->base_region_);
    return true;
}

static inline int labstor_request_map_FindAndRemove(struct labstor_request_map *map, uint32_t key, struct labstor_request **value) {
    uint32_t b = key % map->header_->num_buckets_;
    if(__atomic_load_n(&map->buckets_[b], __ATOMIC_RELAXED) == -1) {
        return false;
    }
    *value = (struct labstor_request*)LABSTOR_REGION_ADD(map->buckets_[b], map->base_region_);
    map->buckets_[b] = -1;
    return true;
}

#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_request_map request_map;
}
uint32_t labstor_request_map::GetSize(uint32_t num_buckets) {
    return labstor_request_map_GetSize_global(num_buckets);
}
uint32_t labstor_request_map::GetSize() {
    return labstor_request_map_GetSize(this);
}
void* labstor_request_map::GetRegion() {
    return labstor_request_map_GetRegion(this);
}
void* labstor_request_map::GetBaseRegion() {
    return labstor_request_map_GetBaseRegion(this);
}
uint32_t labstor_request_map::GetNumBuckets() {
    return labstor_request_map_GetNumBuckets(this);
}
void labstor_request_map::Init(void *base_region, void *region, uint32_t region_size, uint32_t num_buckets) {
    labstor_request_map_Init(this, base_region, region, region_size, num_buckets);
}
void labstor_request_map::Init(void *base_region, void *region, uint32_t region_size) {
    labstor_request_map_Init(this, base_region, region, region_size, 0);
}
void labstor_request_map::Attach(void *base_region, void *region) {
    labstor_request_map_Attach(this, base_region, region);
}
int labstor_request_map::Set(struct labstor_request *rq) {
    return labstor_request_map_Set(this, rq);
}
int labstor_request_map::FindAndRemove(uint32_t key, struct labstor_request* &value) {
    return labstor_request_map_FindAndRemove(this, key, &value);
}
#endif

#endif //LABSTOR_LABSTOR_REQUEST_MAP_H
