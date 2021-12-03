
//{SUFFIX}
//{ARR_SUFFIX}
//{BUCKET_T}
//{BUCKET_HASH}
//{S}
//{S_Atomic}
//{T}
//{GetAtomicKey}
//{GetAtomicKeyRef}
//{GetKey}
//{NullKey}
//{KeyCompare}

#ifndef LABSTOR_UNORDERED_MAP_{SUFFIX}
#define LABSTOR_UNORDERED_MAP_{SUFFIX}

#include <labstor/kernel/types/data_structures/shmem_array_{ARR_SUFFIX}.h>

struct labstor_unordered_map_{SUFFIX} {
    struct labstor_array_{ARR_SUFFIX} buckets_;
    struct labstor_array_{ARR_SUFFIX} overflow_;
};

static inline bool labstor_unordered_map_{SUFFIX}_AtomicSetKeyValue(struct labstor_array_{ARR_SUFFIX} *arr, int i, {BUCKET_T} *bucket);
static inline bool labstor_unordered_map_{SUFFIX}_AtomicGetValueByKey(struct labstor_array_{ARR_SUFFIX} *arr, int i, {S} key, {T} *value);
static inline bool labstor_unordered_map_{SUFFIX}_AtomicNullifyKey(struct labstor_array_{ARR_SUFFIX} *arr, int i, {S} key);

static inline uint32_t labstor_unordered_map_{SUFFIX}_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_{ARR_SUFFIX}_GetSize_global(num_buckets) + labstor_array_{ARR_SUFFIX}_GetSize_global(max_collisions);
}

static inline inline uint32_t labstor_unordered_map_{SUFFIX}_GetSize(struct labstor_unordered_map_{SUFFIX} *map) {
    return labstor_array_{ARR_SUFFIX}_GetSize(&map->buckets_) + labstor_array_{ARR_SUFFIX}_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_{SUFFIX}_GetRegion(struct labstor_unordered_map_{SUFFIX} *map) {
    return labstor_array_{ARR_SUFFIX}_GetRegion(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_{SUFFIX}_GetNumBuckets(struct labstor_unordered_map_{SUFFIX} *map) {
    return labstor_array_{ARR_SUFFIX}_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_{SUFFIX}_GetOverflow(struct labstor_unordered_map_{SUFFIX} *map) {
    return labstor_array_{ARR_SUFFIX}_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_{SUFFIX}_Init(struct labstor_unordered_map_{SUFFIX} *map, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    overflow_region_size = labstor_array_{ARR_SUFFIX}_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_{ARR_SUFFIX}_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_{ARR_SUFFIX}_GetNextSection(&map->buckets_);
    labstor_array_{ARR_SUFFIX}_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_{ARR_SUFFIX}_GetLength(&map->buckets_); ++i) {
        *{GetAtomicKeyRef}(labstor_array_{ARR_SUFFIX}_GetRef(&map->buckets_, i)) = {NullKey}();
    }
    for(i = 0; i < labstor_array_{ARR_SUFFIX}_GetLength(&map->overflow_); ++i) {
        *{GetAtomicKeyRef}(labstor_array_{ARR_SUFFIX}_GetRef(&map->overflow_, i)) = {NullKey}();
    }
}

static inline void labstor_unordered_map_{SUFFIX}_Attach(struct labstor_unordered_map_{SUFFIX} *map, void *region) {
    labstor_array_{ARR_SUFFIX}_Attach(&map->buckets_, region);
    region = labstor_array_{ARR_SUFFIX}_GetNextSection(&map->buckets_);
    labstor_array_{ARR_SUFFIX}_Attach(&map->overflow_, region);
}

static inline bool labstor_unordered_map_{SUFFIX}_Set(struct labstor_unordered_map_{SUFFIX} *map, {BUCKET_T} *bucket) {
    void *bucket_region;
    uint32_t b;
    int i;

    bucket_region = labstor_array_{ARR_SUFFIX}_GetRegion(&map->buckets_);
    b = {BUCKET_HASH}({GetKey}(bucket, bucket_region), bucket_region) % labstor_array_{ARR_SUFFIX}_GetLength(&map->buckets_);
    if(labstor_unordered_map_{SUFFIX}_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_{ARR_SUFFIX}_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_{SUFFIX}_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline bool labstor_unordered_map_{SUFFIX}_Find(struct labstor_unordered_map_{SUFFIX} *map, {S} key, {T} *value) {
    uint32_t b = {BUCKET_HASH}(key, labstor_array_{ARR_SUFFIX}_GetRegion(&map->buckets_)) % labstor_array_{ARR_SUFFIX}_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_{SUFFIX}_AtomicGetValueByKey(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_{ARR_SUFFIX}_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_{SUFFIX}_AtomicGetValueByKey(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline bool labstor_unordered_map_{SUFFIX}_Remove(struct labstor_unordered_map_{SUFFIX} *map, {S} key) {
    int32_t b = {BUCKET_HASH}(key, labstor_array_{ARR_SUFFIX}_GetRegion(&map->buckets_)) % labstor_array_{ARR_SUFFIX}_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_{SUFFIX}_AtomicNullifyKey(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_{ARR_SUFFIX}_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_{SUFFIX}_AtomicNullifyKey(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline bool labstor_unordered_map_{SUFFIX}_AtomicSetKeyValue(struct labstor_array_{ARR_SUFFIX} *arr, int i, {BUCKET_T} *bucket) {
    {S_Atomic} null = {NullKey}();
    if(__atomic_compare_exchange_n(
            {GetAtomicKeyRef}(labstor_array_{ARR_SUFFIX}_GetRef(arr, i)),
            &null,
            {GetAtomicKey}(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_{ARR_SUFFIX}_GetRef(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline bool labstor_unordered_map_{SUFFIX}_AtomicGetValueByKey(struct labstor_array_{ARR_SUFFIX} *arr, int i, {S} key, {T} *value) {
    {BUCKET_T} tmp;
    do {
        tmp = labstor_array_{ARR_SUFFIX}_Get(arr, i);
        if ({KeyCompare}(
                {GetKey}(&tmp, labstor_array_{ARR_SUFFIX}_GetRegion(arr)),
                key)
        ) { return false; }
        *value = {GetValue}(&tmp, labstor_array_{ARR_SUFFIX}_GetRegion(arr));
    } while(!__atomic_compare_exchange_n(
            {GetAtomicKeyRef}(labstor_array_{ARR_SUFFIX}_GetRef(arr, i)),
            {GetAtomicKeyRef}(&tmp),
            {GetAtomicKey}(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline bool labstor_unordered_map_{SUFFIX}_AtomicNullifyKey(struct labstor_array_{ARR_SUFFIX} *arr, int i, {S} key) {
    {BUCKET_T} tmp;
    do {
        tmp = labstor_array_{ARR_SUFFIX}_Get(arr, i);
        if ({KeyCompare}(
                {GetKey}(&tmp, labstor_array_{ARR_SUFFIX}_GetRegion(arr)),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            {GetAtomicKeyRef}(labstor_array_{ARR_SUFFIX}_GetRef(arr, i)),
            {GetAtomicKeyRef}(&tmp),
            {NullKey}(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#endif //LABSTOR_UNORDERED_MAP_{SUFFIX}