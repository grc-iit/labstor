//
// Created by lukemartinlogan on 11/24/21.
//

#ifndef LABSTOR_SHMEM_UNORDERED_MAP_H
#define LABSTOR_SHMEM_UNORDERED_MAP_H

#include "labstor_array.h"

//${SUFFIX}
//${ARR_SUFFIX}
//${BUCKET_T}
//${BUCKET_HASH}
//${S}
//${S_Atomic}
//${T}
//${GetAtomicKey}
//${GetAtomicKeyRef}
//${GetKey}
//${NullKey}
//${KeyCompare}

struct labstor_unordered_map {
    struct labstor_array_${SUFFIX} buckets_;
    struct labstor_array_${SUFFIX} overflow_;
};

static inline uint32_t labstor_unordered_map_GetSize_global_${SUFFIX}(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_GetSize_global_${ARR_SUFFIX}(num_buckets) + labstor_array_GetSize_global_${ARR_SUFFIX}(max_collisions);
}

static inline inline uint32_t labstor_unordered_map_GetSize_${SUFFIX}(struct labstor_unordered_map *map) {
    return labstor_array_GetSize_${ARR_SUFFIX}(&buckets_) + labstor_array_GetSize_${ARR_SUFFIX}(&overflow_);
}

static inline void* labstor_unordered_map_GetRegion_${SUFFIX}(struct labstor_unordered_map *map) {
    return labstor_array_GetRegion_${ARR_SUFFIX}(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_GetNumBuckets_${SUFFIX}(struct labstor_unordered_map *map) {
    return labstor_array_GetLength_${ARR_SUFFIX}(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_GetOverflow_${SUFFIX}(struct labstor_unordered_map *map) {
    return labstor_array_GetLength_${ARR_SUFFIX}(&map->overflow_);
}

static inline void labstor_unordered_map_Init_${SUFFIX}(struct labstor_unordered_map *map, uint32_t element_size, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size = labstor_array_GetSize_global_${ARR_SUFFIX}(max_collisions, element_size);
    uint32_t bucket_region_size = region_size - overflow_region_size;
    labstor_array_Init_${ARR_SUFFIX}(&map->buckets_, region, bucket_region_size);
    region = labstor_array_GetNextSection_${ARR_SUFFIX}(&map->buckets_);
    labstor_array_Init_${ARR_SUFFIX}(&map->overflow_, region, overflow_region_size);

    //Initialize buckets
    for(int i = 0; i < labstor_array_GetLength(&map->buckets_); ++i) { ${GetAtomicKey}(&map->buckets_, i) = ${NullKey}; }
    for(int i = 0; i < labstor_array_GetLength(&map->overflow_); ++i) { ${GetAtomicKey}(&map->overflow_, i) = ${NullKey}; }
}

static inline void labstor_unordered_map_Attach_${SUFFIX}(struct labstor_unordered_map *map, void *region) {
    labstor_array_Attach_${ARR_SUFFIX}(&map->buckets_, region);
    region = labstor_array_GetNextSection_${ARR_SUFFIX}(&map->buckets_);
    labstor_array_Attach_${ARR_SUFFIX}(&map->overflow_, region);
}

static inline bool labstor_unordered_map_Set_${SUFFIX}(struct labstor_unordered_map *map, ${BUCKET_T} *bucket) {
    void *bucket_region = labstor_array_GetRegion_${ARR_SUFFIX}(&map->buckets_);
    uint32_t b = ${BUCKET_HASH}(${GetKey}(bucket, bucket_region), bucket_region) % labstor_array_GetLength_${ARR_SUFFIX}(&map->buckets_);
    if(labstor_unordered_map_AtomicSetKeyValue_${SUFFIX}(&map->buckets_, b, bucket)) {
        return true;
    }
    for(int i = 0; i < labstor_array_GetLength_${SUFFIX}(map->overflow_); ++i) {
        if(labstor_unordered_map_AtomicSetKeyValue_${SUFFIX}(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline bool labstor_unordered_map_Find_${SUFFIX}(struct labstor_unordered_map *map, ${S} key, ${T} *value) {
    uint32_t b = ${BUCKET_HASH}(key, labstor_array_GetRegion_${ARR_SUFFIX}(&map->buckets_)) % labstor_array_GetLength_${ARR_SUFFIX}(&map->buckets_);

    //Check the primary map first
    if(labstor_unordered_map_AtomicGetValueByKey_${SUFFIX}(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(int i = 0; i < labstor_array_GetLength_${ARR_SUFFIX}(&map->overflow_); ++i) {
        if(labstor_unordered_map_AtomicGetValueByKey_${SUFFIX}(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline bool labstor_unordered_map_Remove_${SUFFIX}(struct labstor_unordered_map *map, ${S} key) {
    int32_t b = ${BUCKET_HASH}(key, labstor_array_GetRegion(&map->buckets_)) % labstor_array_GetLength_${ARR_SUFFIX}(&map->buckets_);

    //Check the primary map first
    if(labstor_unordered_map_AtomicNullifyKey_${SUFFIX}(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(int i = 0; i < labstor_array_GetLength_${ARR_SUFFIX}(&map->overflow_); ++i) {
        if(labstor_unordered_map_AtomicNullifyKey_${SUFFIX}(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline bool labstor_unordered_map_AtomicSetKeyValue_${SUFFIX}(struct labstor_array *arr, int i, ${BUCKET_T} *bucket) {
    S_Atomic null = NullKey;
    if(__atomic_compare_exchange_n(
            ${GetAtomicKey}(labstor_array_GetRef(arr, i)),
            &null,
            GetMarkedAtomicKey(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        arr[i] = bucket;
        return true;
    }
    return false;
}

static inline bool labstor_unordered_map_AtomicGetValueByKey_${SUFFIX}(struct labstor_array *arr, int i, ${S} *key, ${T} *value) {
    ${BUCKET_T} tmp;
    do {
        tmp = arr[i];
        if (${KeyCompare}(${GetKey}(&tmp, labstor_array_GetRegion(&map->buckets_)), key)) { return false; }
        value = ${GetValue}(&tmp, labstor_array_GetRegion_${ARR_SUFFIX}(map->buckets_));
    } while(!__atomic_compare_exchange_n(
            ${GetAtomicKeyRef}(labstor_array_GetRef(arr, i)),
            ${GetAtomicKeyRef}(&tmp),
            ${GetAtomicKeyRef}(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline bool labstor_unordered_map_AtomicNullifyKey_${SUFFIX}(struct labstor_array *arr, int i, ${S} &key) {
    ${BUCKET_T} tmp;
    do {
        tmp = arr[i];
        if (${KeyCompare}(${GetKey}(&tmp, labstor_array_GetRegion(&map->buckets_)), key)) { return false; }
    } while(!__atomic_compare_exchange_n(!__atomic_compare_exchange_n(
            ${GetAtomicKeyRef}(labstor_array_GetRef(arr, i)),
            ${GetAtomicKeyRef}(&tmp),
            ${NullKey},
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#endif //LABSTOR_SHMEM_UNORDERED_MAP_H
