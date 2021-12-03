
//labstor::ipc::string: The key's real type
//labstor_string: The semantic name of type S
//labstor_off_t: How the key is stored in the bucket
//uint32_t: The value's type
//uint32_t: The semantic name of type T
//labstor_string_map_bucket: The semantic name of the bucket type
//labstor_string_map_bucket_hash(uint32_t key, void *region)
//labstor_string_map_bucket_GetAtomicKey()
//labstor_string_map_bucket_GetAtomicKeyRef()
//labstor_string_map_bucket_GetKey
//labstor_string_map_bucket_NullKey
//labstor_string_map_bucket_KeyCompare

#ifndef LABSTOR_UNORDERED_MAP_labstor_string_uint32_t
#define LABSTOR_UNORDERED_MAP_labstor_string_uint32_t

#include <labstor/types/data_structures/array/shmem_array_labstor_string_map_bucket.h>

struct labstor_unordered_map_labstor_string_uint32_t {
    struct labstor_array_labstor_string_map_bucket buckets_;
    struct labstor_array_labstor_string_map_bucket overflow_;
};

static inline bool labstor_unordered_map_labstor_string_uint32_t_AtomicSetKeyValue(struct labstor_array_labstor_string_map_bucket *arr, int i, struct labstor_string_map_bucket *bucket);
static inline bool labstor_unordered_map_labstor_string_uint32_t_AtomicGetValueByKey(struct labstor_array_labstor_string_map_bucket *arr, int i, labstor::ipc::string key, uint32_t *value);
static inline bool labstor_unordered_map_labstor_string_uint32_t_AtomicNullifyKey(struct labstor_array_labstor_string_map_bucket *arr, int i, labstor::ipc::string key);

static inline uint32_t labstor_unordered_map_labstor_string_uint32_t_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_labstor_string_map_bucket_GetSize_global(num_buckets) + labstor_array_labstor_string_map_bucket_GetSize_global(max_collisions);
}

static inline uint32_t labstor_unordered_map_labstor_string_uint32_t_GetSize(struct labstor_unordered_map_labstor_string_uint32_t *map) {
    return labstor_array_labstor_string_map_bucket_GetSize(&map->buckets_) + labstor_array_labstor_string_map_bucket_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_labstor_string_uint32_t_GetRegion(struct labstor_unordered_map_labstor_string_uint32_t *map) {
    return labstor_array_labstor_string_map_bucket_GetRegion(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_labstor_string_uint32_t_GetNumBuckets(struct labstor_unordered_map_labstor_string_uint32_t *map) {
    return labstor_array_labstor_string_map_bucket_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_labstor_string_uint32_t_GetOverflow(struct labstor_unordered_map_labstor_string_uint32_t *map) {
    return labstor_array_labstor_string_map_bucket_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_labstor_string_uint32_t_Init(struct labstor_unordered_map_labstor_string_uint32_t *map, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    overflow_region_size = labstor_array_labstor_string_map_bucket_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_labstor_string_map_bucket_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_labstor_string_map_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_string_map_bucket_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_labstor_string_map_bucket_GetLength(&map->buckets_); ++i) {
        *labstor_string_map_bucket_GetAtomicKeyRef(labstor_array_labstor_string_map_bucket_GetPtr(&map->buckets_, i)) = labstor_string_map_bucket_NullKey();
    }
    for(i = 0; i < labstor_array_labstor_string_map_bucket_GetLength(&map->overflow_); ++i) {
        *labstor_string_map_bucket_GetAtomicKeyRef(labstor_array_labstor_string_map_bucket_GetPtr(&map->overflow_, i)) = labstor_string_map_bucket_NullKey();
    }
}

static inline void labstor_unordered_map_labstor_string_uint32_t_Attach(struct labstor_unordered_map_labstor_string_uint32_t *map, void *region) {
    labstor_array_labstor_string_map_bucket_Attach(&map->buckets_, region);
    region = labstor_array_labstor_string_map_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_string_map_bucket_Attach(&map->overflow_, region);
}

static inline bool labstor_unordered_map_labstor_string_uint32_t_Set(struct labstor_unordered_map_labstor_string_uint32_t *map, struct labstor_string_map_bucket *bucket) {
    void *bucket_region;
    uint32_t b;
    int i;

    bucket_region = labstor_array_labstor_string_map_bucket_GetRegion(&map->buckets_);
    b = labstor_string_map_bucket_hash(labstor_string_map_bucket_GetKey(bucket, bucket_region), bucket_region) % labstor_array_labstor_string_map_bucket_GetLength(&map->buckets_);
    if(labstor_unordered_map_labstor_string_uint32_t_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_labstor_string_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_labstor_string_uint32_t_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline bool labstor_unordered_map_labstor_string_uint32_t_Find(struct labstor_unordered_map_labstor_string_uint32_t *map, labstor::ipc::string key, uint32_t *value) {
    uint32_t b = labstor_string_map_bucket_hash(key, labstor_array_labstor_string_map_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_string_map_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_labstor_string_uint32_t_AtomicGetValueByKey(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_labstor_string_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_labstor_string_uint32_t_AtomicGetValueByKey(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline bool labstor_unordered_map_labstor_string_uint32_t_Remove(struct labstor_unordered_map_labstor_string_uint32_t *map, labstor::ipc::string key) {
    int32_t b = labstor_string_map_bucket_hash(key, labstor_array_labstor_string_map_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_string_map_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_labstor_string_uint32_t_AtomicNullifyKey(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_labstor_string_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_labstor_string_uint32_t_AtomicNullifyKey(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline bool labstor_unordered_map_labstor_string_uint32_t_AtomicSetKeyValue(struct labstor_array_labstor_string_map_bucket *arr, int i, struct labstor_string_map_bucket *bucket) {
    labstor_off_t null = labstor_string_map_bucket_NullKey();
    if(__atomic_compare_exchange_n(
            labstor_string_map_bucket_GetAtomicKeyRef(labstor_array_labstor_string_map_bucket_GetPtr(arr, i)),
            &null,
            labstor_string_map_bucket_GetAtomicKey(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_labstor_string_map_bucket_GetPtr(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline bool labstor_unordered_map_labstor_string_uint32_t_AtomicGetValueByKey(struct labstor_array_labstor_string_map_bucket *arr, int i, labstor::ipc::string key, uint32_t *value) {
    struct labstor_string_map_bucket tmp;
    do {
        tmp = labstor_array_labstor_string_map_bucket_Get(arr, i);
        if (labstor_string_map_bucket_KeyCompare(
                labstor_string_map_bucket_GetKey(&tmp, labstor_array_labstor_string_map_bucket_GetRegion(arr)),
                key)
        ) { return false; }
        *value = labstor_string_map_bucket_GetValue(&tmp, labstor_array_labstor_string_map_bucket_GetRegion(arr));
    } while(!__atomic_compare_exchange_n(
            labstor_string_map_bucket_GetAtomicKeyRef(labstor_array_labstor_string_map_bucket_GetPtr(arr, i)),
            labstor_string_map_bucket_GetAtomicKeyRef(&tmp),
            labstor_string_map_bucket_GetAtomicKey(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline bool labstor_unordered_map_labstor_string_uint32_t_AtomicNullifyKey(struct labstor_array_labstor_string_map_bucket *arr, int i, labstor::ipc::string key) {
    struct labstor_string_map_bucket tmp;
    do {
        tmp = labstor_array_labstor_string_map_bucket_Get(arr, i);
        if (labstor_string_map_bucket_KeyCompare(
                labstor_string_map_bucket_GetKey(&tmp, labstor_array_labstor_string_map_bucket_GetRegion(arr)),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            labstor_string_map_bucket_GetAtomicKeyRef(labstor_array_labstor_string_map_bucket_GetPtr(arr, i)),
            labstor_string_map_bucket_GetAtomicKeyRef(&tmp),
            labstor_string_map_bucket_NullKey(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
namespace labstor::ipc {

class unordered_map_labstor_string_uint32_t : protected labstor_unordered_map_labstor_string_uint32_t, public shmem_type {
public:
    static uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions) {
        return labstor_unordered_map_labstor_string_uint32_t_GetSize_global(num_buckets, max_collisions);
    }
    inline uint32_t GetSize() {
        return labstor_unordered_map_labstor_string_uint32_t_GetSize(this);
    }
    inline void* GetRegion() {
        return labstor_unordered_map_labstor_string_uint32_t_GetRegion(this);
    }
    inline uint32_t GetNumBuckets() {
        return labstor_unordered_map_labstor_string_uint32_t_GetNumBuckets(this);
    }
    inline uint32_t GetOverflow() {
        return labstor_unordered_map_labstor_string_uint32_t_GetOverflow(this);
    }

    inline void Init(void *region, uint32_t region_size, uint32_t max_collisions) {
        labstor_unordered_map_labstor_string_uint32_t_Init(this, region, region_size, max_collisions);
    }
    inline void Attach(void *region) {
        labstor_unordered_map_labstor_string_uint32_t_Attach(this, region);
    }

    inline bool Set(struct labstor_string_map_bucket &bucket) {
        return labstor_unordered_map_labstor_string_uint32_t_Set(this, &bucket);
    }

    inline bool Find(labstor::ipc::string key, uint32_t &value) {
        return labstor_unordered_map_labstor_string_uint32_t_Find(this, key, &value);
    }

    inline bool Remove(labstor::ipc::string key) {
        return labstor_unordered_map_labstor_string_uint32_t_Remove(this, key);
    }

    inline uint32_t operator [](labstor::ipc::string key) {
        uint32_t value;
        if(Find(key, value)) {
            return value;
        }
        throw INVALID_UNORDERED_MAP_KEY.format();
    }
};

}

#endif

#endif //LABSTOR_UNORDERED_MAP_labstor_string_uint32_t