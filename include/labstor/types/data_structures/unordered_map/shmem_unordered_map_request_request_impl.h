
//struct labstor_request*: The key's real type
//request: The semantic name of type S
//labstor_off_t: How the key is stored in the bucket
//struct labstor_request*: The value's type
//request: The semantic name of type T
//labstor_request_bucket: The semantic name of the bucket type
//labstor_request_bucket_hash(struct labstor_request* key, void *region)
//labstor_request_bucket_GetAtomicKey()
//labstor_request_bucket_GetAtomicKeyRef()
//labstor_request_bucket_GetKey
//labstor_request_bucket_NullKey
//labstor_request_bucket_KeyCompare

#ifndef LABSTOR_UNORDERED_MAP_request_request
#define LABSTOR_UNORDERED_MAP_request_request

#include <labstor/kernel/types/data_structures/shmem_array_labstor_request_bucket.h>

struct labstor_unordered_map_request_request {
    struct labstor_array_labstor_request_bucket buckets_;
    struct labstor_array_labstor_request_bucket overflow_;
};

static inline bool labstor_unordered_map_request_request_AtomicSetKeyValue(struct labstor_array_labstor_request_bucket *arr, int i, struct labstor_request_bucket *bucket);
static inline bool labstor_unordered_map_request_request_AtomicGetValueByKey(struct labstor_array_labstor_request_bucket *arr, int i, struct labstor_request* key, struct labstor_request* *value);
static inline bool labstor_unordered_map_request_request_AtomicNullifyKey(struct labstor_array_labstor_request_bucket *arr, int i, struct labstor_request* key);

static inline uint32_t labstor_unordered_map_request_request_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_labstor_request_bucket_GetSize_global(num_buckets) + labstor_array_labstor_request_bucket_GetSize_global(max_collisions);
}

static inline inline uint32_t labstor_unordered_map_request_request_GetSize(struct labstor_unordered_map_request_request *map) {
    return labstor_array_labstor_request_bucket_GetSize(&map->buckets_) + labstor_array_labstor_request_bucket_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_request_request_GetRegion(struct labstor_unordered_map_request_request *map) {
    return labstor_array_labstor_request_bucket_GetRegion(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_request_request_GetNumBuckets(struct labstor_unordered_map_request_request *map) {
    return labstor_array_labstor_request_bucket_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_request_request_GetOverflow(struct labstor_unordered_map_request_request *map) {
    return labstor_array_labstor_request_bucket_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_request_request_Init(struct labstor_unordered_map_request_request *map, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    overflow_region_size = labstor_array_labstor_request_bucket_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_labstor_request_bucket_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_labstor_request_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_request_bucket_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_labstor_request_bucket_GetLength(&map->buckets_); ++i) {
        *labstor_request_bucket_GetAtomicKeyRef(labstor_array_labstor_request_bucket_GetPtr(&map->buckets_, i)) = labstor_request_bucket_NullKey();
    }
    for(i = 0; i < labstor_array_labstor_request_bucket_GetLength(&map->overflow_); ++i) {
        *labstor_request_bucket_GetAtomicKeyRef(labstor_array_labstor_request_bucket_GetPtr(&map->overflow_, i)) = labstor_request_bucket_NullKey();
    }
}

static inline void labstor_unordered_map_request_request_Attach(struct labstor_unordered_map_request_request *map, void *region) {
    labstor_array_labstor_request_bucket_Attach(&map->buckets_, region);
    region = labstor_array_labstor_request_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_request_bucket_Attach(&map->overflow_, region);
}

static inline bool labstor_unordered_map_request_request_Set(struct labstor_unordered_map_request_request *map, struct labstor_request_bucket *bucket) {
    void *bucket_region;
    uint32_t b;
    int i;

    bucket_region = labstor_array_labstor_request_bucket_GetRegion(&map->buckets_);
    b = labstor_request_bucket_hash(labstor_request_bucket_GetKey(bucket, bucket_region), bucket_region) % labstor_array_labstor_request_bucket_GetLength(&map->buckets_);
    if(labstor_unordered_map_request_request_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_labstor_request_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_request_request_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline bool labstor_unordered_map_request_request_Find(struct labstor_unordered_map_request_request *map, struct labstor_request* key, struct labstor_request* *value) {
    uint32_t b = labstor_request_bucket_hash(key, labstor_array_labstor_request_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_request_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_request_request_AtomicGetValueByKey(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_labstor_request_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_request_request_AtomicGetValueByKey(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline bool labstor_unordered_map_request_request_Remove(struct labstor_unordered_map_request_request *map, struct labstor_request* key) {
    int32_t b = labstor_request_bucket_hash(key, labstor_array_labstor_request_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_request_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_request_request_AtomicNullifyKey(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_labstor_request_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_request_request_AtomicNullifyKey(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline bool labstor_unordered_map_request_request_AtomicSetKeyValue(struct labstor_array_labstor_request_bucket *arr, int i, struct labstor_request_bucket *bucket) {
    labstor_off_t null = labstor_request_bucket_NullKey();
    if(__atomic_compare_exchange_n(
            labstor_request_bucket_GetAtomicKeyRef(labstor_array_labstor_request_bucket_GetPtr(arr, i)),
            &null,
            labstor_request_bucket_GetAtomicKey(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_labstor_request_bucket_GetPtr(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline bool labstor_unordered_map_request_request_AtomicGetValueByKey(struct labstor_array_labstor_request_bucket *arr, int i, struct labstor_request* key, struct labstor_request* *value) {
    struct labstor_request_bucket tmp;
    do {
        tmp = labstor_array_labstor_request_bucket_Get(arr, i);
        if (labstor_request_bucket_KeyCompare(
                labstor_request_bucket_GetKey(&tmp, labstor_array_labstor_request_bucket_GetRegion(arr)),
                key)
        ) { return false; }
        *value = labstor_request_bucket_GetValue(&tmp, labstor_array_labstor_request_bucket_GetRegion(arr));
    } while(!__atomic_compare_exchange_n(
            labstor_request_bucket_GetAtomicKeyRef(labstor_array_labstor_request_bucket_GetPtr(arr, i)),
            labstor_request_bucket_GetAtomicKeyRef(&tmp),
            labstor_request_bucket_GetAtomicKey(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline bool labstor_unordered_map_request_request_AtomicNullifyKey(struct labstor_array_labstor_request_bucket *arr, int i, struct labstor_request* key) {
    struct labstor_request_bucket tmp;
    do {
        tmp = labstor_array_labstor_request_bucket_Get(arr, i);
        if (labstor_request_bucket_KeyCompare(
                labstor_request_bucket_GetKey(&tmp, labstor_array_labstor_request_bucket_GetRegion(arr)),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            labstor_request_bucket_GetAtomicKeyRef(labstor_array_labstor_request_bucket_GetPtr(arr, i)),
            labstor_request_bucket_GetAtomicKeyRef(&tmp),
            labstor_request_bucket_NullKey(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#ifdef __cplusplus

namespace labstor::ipc {

class unordered_map_request_request; : protected labstor_unordered_map_request_request, public shmem_type {
public:
    static uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions) {
        return labstor_unordered_map_request_request_GetSize_global(num_buckets, max_collisions);
    }
    inline uint32_t GetSize() {
        return labstor_unordered_map_request_request_GetSize(this);
    }
    inline void* GetRegion() {
        return labstor_unordered_map_request_request_GetRegion(this);
    }
    inline uint32_t GetNumBuckets() {
        return labstor_unordered_map_request_request_GetNumBuckets(this);
    }
    inline uint32_t GetOverflow() {
        return labstor_unordered_map_request_request_GetOverflow(this);
    }

    inline void Init(void *region, uint32_t region_size, uint32_t max_collisions) {
        labstor_unordered_map_request_request_Init(this, region, region_size, max_collisions);
    }
    inline void Attach(void *region) {
        labstor_unordered_map_request_request_Attach(this, region);
    }

    inline bool Set(struct labstor_request_bucket &bucket) {
        return labstor_unordered_map_request_request_Set(this, &bucket);
    }

    inline bool Find(struct labstor_request* key, struct labstor_request* &value) {
        return labstor_unordered_map_request_request_Find(this, key, &value);
    }

    inline bool Remove(struct labstor_request* key) {
        return labstor_unordered_map_request_request_Remove(this, key);
    }

    inline T operator [](struct labstor_request* key) {
        T value;
        if(Find(key, value)) {
            return value;
        }
        throw INVALID_UNORDERED_MAP_KEY.format();
    }
};

}

#endif

#endif

#endif //LABSTOR_UNORDERED_MAP_request_request