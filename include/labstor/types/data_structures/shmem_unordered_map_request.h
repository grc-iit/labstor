
//request
//request_map_bucket
//struct labstor_request_map_bucket
//labstor_request_map_bucket_hash
//labstor_off_t
//labstor_off_t
//struct labstor_request*
//labstor_request_map_bucket_GetAtomicKey
//labstor_request_map_bucket_GetAtomicKeyRef
//labstor_request_map_bucket_GetKey
//labstor_request_map_bucket_NullKey
//labstor_request_map_bucket_KeyCompare

#ifndef LABSTOR_UNORDERED_MAP_request
#define LABSTOR_UNORDERED_MAP_request

#include <labstor/types/data_structures/array/shmem_array_request_map_bucket.h>

struct labstor_unordered_map_request {
    struct labstor_array_request_map_bucket buckets_;
    struct labstor_array_request_map_bucket overflow_;
};

static inline bool labstor_unordered_map_request_AtomicSetKeyValue(struct labstor_array_request_map_bucket *arr, int i, struct labstor_request_map_bucket *bucket);
static inline bool labstor_unordered_map_request_AtomicGetValueByKey(struct labstor_array_request_map_bucket *arr, int i, labstor_off_t key, struct labstor_request* *value);
static inline bool labstor_unordered_map_request_AtomicNullifyKey(struct labstor_array_request_map_bucket *arr, int i, labstor_off_t key);

static inline uint32_t labstor_unordered_map_request_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_request_map_bucket_GetSize_global(num_buckets) + labstor_array_request_map_bucket_GetSize_global(max_collisions);
}

static inline inline uint32_t labstor_unordered_map_request_GetSize(struct labstor_unordered_map_request *map) {
    return labstor_array_request_map_bucket_GetSize(&map->buckets_) + labstor_array_request_map_bucket_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_request_GetRegion(struct labstor_unordered_map_request *map) {
    return labstor_array_request_map_bucket_GetRegion(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_request_GetNumBuckets(struct labstor_unordered_map_request *map) {
    return labstor_array_request_map_bucket_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_request_GetOverflow(struct labstor_unordered_map_request *map) {
    return labstor_array_request_map_bucket_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_request_Init(struct labstor_unordered_map_request *map, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    overflow_region_size = labstor_array_request_map_bucket_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_request_map_bucket_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_request_map_bucket_GetNextSection(&map->buckets_);
    labstor_array_request_map_bucket_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_request_map_bucket_GetLength(&map->buckets_); ++i) {
        *labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_request_map_bucket_GetRef(&map->buckets_, i)) = labstor_request_map_bucket_NullKey();
    }
    for(i = 0; i < labstor_array_request_map_bucket_GetLength(&map->overflow_); ++i) {
        *labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_request_map_bucket_GetRef(&map->overflow_, i)) = labstor_request_map_bucket_NullKey();
    }
}

static inline void labstor_unordered_map_request_Attach(struct labstor_unordered_map_request *map, void *region) {
    labstor_array_request_map_bucket_Attach(&map->buckets_, region);
    region = labstor_array_request_map_bucket_GetNextSection(&map->buckets_);
    labstor_array_request_map_bucket_Attach(&map->overflow_, region);
}

static inline bool labstor_unordered_map_request_Set(struct labstor_unordered_map_request *map, struct labstor_request_map_bucket *bucket) {
    void *bucket_region;
    uint32_t b;
    int i;

    bucket_region = labstor_array_request_map_bucket_GetRegion(&map->buckets_);
    b = labstor_request_map_bucket_hash(labstor_request_map_bucket_GetKey(bucket, bucket_region), bucket_region) % labstor_array_request_map_bucket_GetLength(&map->buckets_);
    if(labstor_unordered_map_request_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_request_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_request_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline bool labstor_unordered_map_request_Find(struct labstor_unordered_map_request *map, labstor_off_t key, struct labstor_request* *value) {
    uint32_t b = labstor_request_map_bucket_hash(key, labstor_array_request_map_bucket_GetRegion(&map->buckets_)) % labstor_array_request_map_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_request_AtomicGetValueByKey(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_request_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_request_AtomicGetValueByKey(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline bool labstor_unordered_map_request_Remove(struct labstor_unordered_map_request *map, labstor_off_t key) {
    int32_t b = labstor_request_map_bucket_hash(key, labstor_array_request_map_bucket_GetRegion(&map->buckets_)) % labstor_array_request_map_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_request_AtomicNullifyKey(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_request_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_request_AtomicNullifyKey(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline bool labstor_unordered_map_request_AtomicSetKeyValue(struct labstor_array_request_map_bucket *arr, int i, struct labstor_request_map_bucket *bucket) {
    labstor_off_t null = labstor_request_map_bucket_NullKey();
    if(__atomic_compare_exchange_n(
            labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_request_map_bucket_GetRef(arr, i)),
            &null,
            labstor_request_map_bucket_GetAtomicKey(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_request_map_bucket_GetRef(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline bool labstor_unordered_map_request_AtomicGetValueByKey(struct labstor_array_request_map_bucket *arr, int i, labstor_off_t key, struct labstor_request* *value) {
    struct labstor_request_map_bucket tmp;
    do {
        tmp = labstor_array_request_map_bucket_Get(arr, i);
        if (labstor_request_map_bucket_KeyCompare(
                labstor_request_map_bucket_GetKey(&tmp, labstor_array_request_map_bucket_GetRegion(arr)),
                key)
        ) { return false; }
        *value = labstor_request_map_bucket_GetValue(&tmp, labstor_array_request_map_bucket_GetRegion(arr));
    } while(!__atomic_compare_exchange_n(
            labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_request_map_bucket_GetRef(arr, i)),
            labstor_request_map_bucket_GetAtomicKeyRef(&tmp),
            labstor_request_map_bucket_GetAtomicKey(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline bool labstor_unordered_map_request_AtomicNullifyKey(struct labstor_array_request_map_bucket *arr, int i, labstor_off_t key) {
    struct labstor_request_map_bucket tmp;
    do {
        tmp = labstor_array_request_map_bucket_Get(arr, i);
        if (labstor_request_map_bucket_KeyCompare(
                labstor_request_map_bucket_GetKey(&tmp, labstor_array_request_map_bucket_GetRegion(arr)),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_request_map_bucket_GetRef(arr, i)),
            labstor_request_map_bucket_GetAtomicKeyRef(&tmp),
            labstor_request_map_bucket_NullKey(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#endif //LABSTOR_UNORDERED_MAP_request