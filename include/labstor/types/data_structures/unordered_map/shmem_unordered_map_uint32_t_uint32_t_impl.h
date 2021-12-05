
//uint32_t: The key's real type
//uint32_t: The semantic name of type S
//uint32_t: How the key is stored in the bucket
//uint32_t: The value's type
//uint32_t: The semantic name of type T
//labstor_uint32_t_uint32_t_bucket: The semantic name of the bucket type
//labstor_uint32_t_uint32_t_bucket_hash(uint32_t key, void *region)
//labstor_uint32_t_uint32_t_bucket_GetAtomicKey()
//labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef()
//labstor_uint32_t_uint32_t_bucket_GetKey
//labstor_uint32_t_uint32_t_bucket_NullKey
//labstor_uint32_t_uint32_t_bucket_KeyCompare

#ifndef LABSTOR_UNORDERED_MAP_uint32_t_uint32_t
#define LABSTOR_UNORDERED_MAP_uint32_t_uint32_t

#include <labstor/types/data_structures/array/shmem_array_labstor_uint32_t_uint32_t_bucket.h>

#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
#endif

#ifdef __cplusplus
struct labstor_unordered_map_uint32_t_uint32_t : public labstor::shmem_type {
#else
struct labstor_unordered_map_uint32_t_uint32_t {
#endif
    void *base_region_;
    struct labstor_array_labstor_uint32_t_uint32_t_bucket buckets_;
    struct labstor_array_labstor_uint32_t_uint32_t_bucket overflow_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void* GetBaseRegion();
    inline uint32_t GetNumBuckets();
    inline uint32_t GetOverflow();
    inline void Init(void *base_region, void *region, uint32_t region_size, uint32_t max_collisions);
    inline void Attach(void *base_region, void *region);
    inline int Set(struct labstor_uint32_t_uint32_t_bucket &bucket);
    inline int Find(uint32_t key, uint32_t &value);
    inline int Remove(uint32_t key);
    inline uint32_t operator [](uint32_t key) {
        uint32_t value;
        if(Find(key, value)) {
            return value;
        }
        throw labstor::INVALID_UNORDERED_MAP_KEY.format();
    }
#endif
};

static inline int labstor_unordered_map_uint32_t_uint32_t_AtomicSetKeyValue(struct labstor_array_labstor_uint32_t_uint32_t_bucket *arr, int i, struct labstor_uint32_t_uint32_t_bucket *bucket);
static inline int labstor_unordered_map_uint32_t_uint32_t_AtomicGetValueByKey(void *base_region, struct labstor_array_labstor_uint32_t_uint32_t_bucket *arr, int i, uint32_t key, uint32_t *value);
static inline int labstor_unordered_map_uint32_t_uint32_t_AtomicNullifyKey(void *base_region, struct labstor_array_labstor_uint32_t_uint32_t_bucket *arr, int i, uint32_t key);

static inline uint32_t labstor_unordered_map_uint32_t_uint32_t_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_labstor_uint32_t_uint32_t_bucket_GetSize_global(num_buckets) + labstor_array_labstor_uint32_t_uint32_t_bucket_GetSize_global(max_collisions);
}

static inline uint32_t labstor_unordered_map_uint32_t_uint32_t_GetSize(struct labstor_unordered_map_uint32_t_uint32_t *map) {
    return labstor_array_labstor_uint32_t_uint32_t_bucket_GetSize(&map->buckets_) + labstor_array_labstor_uint32_t_uint32_t_bucket_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_uint32_t_uint32_t_GetRegion(struct labstor_unordered_map_uint32_t_uint32_t *map) {
    return labstor_array_labstor_uint32_t_uint32_t_bucket_GetRegion(&map->buckets_);
}

static inline void* labstor_unordered_map_uint32_t_uint32_t_GetBaseRegion(struct labstor_unordered_map_uint32_t_uint32_t *map) {
    return map->base_region_;
}

static inline uint32_t labstor_unordered_map_uint32_t_uint32_t_GetNumBuckets(struct labstor_unordered_map_uint32_t_uint32_t *map) {
    return labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_uint32_t_uint32_t_GetOverflow(struct labstor_unordered_map_uint32_t_uint32_t *map) {
    return labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_uint32_t_uint32_t_Init(
        struct labstor_unordered_map_uint32_t_uint32_t *map, void *base_region,
        void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    map->base_region_ = base_region;
    overflow_region_size = labstor_array_labstor_uint32_t_uint32_t_bucket_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_labstor_uint32_t_uint32_t_bucket_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_labstor_uint32_t_uint32_t_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_uint32_t_uint32_t_bucket_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->buckets_); ++i) {
        *labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef(labstor_array_labstor_uint32_t_uint32_t_bucket_GetPtr(&map->buckets_, i)) = labstor_uint32_t_uint32_t_bucket_NullKey();
    }
    for(i = 0; i < labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->overflow_); ++i) {
        *labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef(labstor_array_labstor_uint32_t_uint32_t_bucket_GetPtr(&map->overflow_, i)) = labstor_uint32_t_uint32_t_bucket_NullKey();
    }
}

static inline void labstor_unordered_map_uint32_t_uint32_t_Attach(
        struct labstor_unordered_map_uint32_t_uint32_t *map, void *base_region, void *region) {
    map->base_region_ = base_region;
    labstor_array_labstor_uint32_t_uint32_t_bucket_Attach(&map->buckets_, region);
    region = labstor_array_labstor_uint32_t_uint32_t_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_uint32_t_uint32_t_bucket_Attach(&map->overflow_, region);
}

static inline int labstor_unordered_map_uint32_t_uint32_t_Set(struct labstor_unordered_map_uint32_t_uint32_t *map, struct labstor_uint32_t_uint32_t_bucket *bucket) {
    uint32_t b;
    int i;

    b = labstor_uint32_t_uint32_t_bucket_hash(labstor_uint32_t_uint32_t_bucket_GetKey(bucket, map->base_region_), map->base_region_) % labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->buckets_);
    if(labstor_unordered_map_uint32_t_uint32_t_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_uint32_t_uint32_t_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline int labstor_unordered_map_uint32_t_uint32_t_Find(struct labstor_unordered_map_uint32_t_uint32_t *map, uint32_t key, uint32_t *value) {
    uint32_t b = labstor_uint32_t_uint32_t_bucket_hash(key, map->base_region_) % labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_uint32_t_uint32_t_AtomicGetValueByKey(map->base_region_, &map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_uint32_t_uint32_t_AtomicGetValueByKey(map->base_region_, &map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline int labstor_unordered_map_uint32_t_uint32_t_Remove(struct labstor_unordered_map_uint32_t_uint32_t *map, uint32_t key) {
    int32_t b = labstor_uint32_t_uint32_t_bucket_hash(key, map->base_region_) % labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_uint32_t_uint32_t_AtomicNullifyKey(map->base_region_, &map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_labstor_uint32_t_uint32_t_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_uint32_t_uint32_t_AtomicNullifyKey(map->base_region_, &map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline int labstor_unordered_map_uint32_t_uint32_t_AtomicSetKeyValue(struct labstor_array_labstor_uint32_t_uint32_t_bucket *arr, int i, struct labstor_uint32_t_uint32_t_bucket *bucket) {
    uint32_t null = labstor_uint32_t_uint32_t_bucket_NullKey();
    if(__atomic_compare_exchange_n(
            labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef(labstor_array_labstor_uint32_t_uint32_t_bucket_GetPtr(arr, i)),
            &null,
            labstor_uint32_t_uint32_t_bucket_GetAtomicKey(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_labstor_uint32_t_uint32_t_bucket_GetPtr(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline int labstor_unordered_map_uint32_t_uint32_t_AtomicGetValueByKey(void *base_region, struct labstor_array_labstor_uint32_t_uint32_t_bucket *arr, int i, uint32_t key, uint32_t *value) {
    struct labstor_uint32_t_uint32_t_bucket tmp;
    do {
        tmp = labstor_array_labstor_uint32_t_uint32_t_bucket_Get(arr, i);
        if (!labstor_uint32_t_uint32_t_bucket_KeyCompare(
                labstor_uint32_t_uint32_t_bucket_GetKey(&tmp, base_region),
                key)
        ) { return false; }
        *value = labstor_uint32_t_uint32_t_bucket_GetValue(&tmp, base_region);
    } while(!__atomic_compare_exchange_n(
            labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef(labstor_array_labstor_uint32_t_uint32_t_bucket_GetPtr(arr, i)),
            labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef(&tmp),
            labstor_uint32_t_uint32_t_bucket_GetAtomicKey(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline int labstor_unordered_map_uint32_t_uint32_t_AtomicNullifyKey(void *base_region, struct labstor_array_labstor_uint32_t_uint32_t_bucket *arr, int i, uint32_t key) {
    struct labstor_uint32_t_uint32_t_bucket tmp;
    do {
        tmp = labstor_array_labstor_uint32_t_uint32_t_bucket_Get(arr, i);
        if (!labstor_uint32_t_uint32_t_bucket_KeyCompare(
                labstor_uint32_t_uint32_t_bucket_GetKey(&tmp, base_region),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef(labstor_array_labstor_uint32_t_uint32_t_bucket_GetPtr(arr, i)),
            labstor_uint32_t_uint32_t_bucket_GetAtomicKeyRef(&tmp),
            labstor_uint32_t_uint32_t_bucket_NullKey(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#ifdef __cplusplus

namespace labstor::ipc {
    typedef labstor_unordered_map_uint32_t_uint32_t unordered_map_uint32_t_uint32_t;
}
uint32_t labstor_unordered_map_uint32_t_uint32_t::GetSize(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_unordered_map_uint32_t_uint32_t_GetSize_global(num_buckets, max_collisions);
}
uint32_t labstor_unordered_map_uint32_t_uint32_t::GetSize() {
    return labstor_unordered_map_uint32_t_uint32_t_GetSize(this);
}
void* labstor_unordered_map_uint32_t_uint32_t::GetRegion() {
    return labstor_unordered_map_uint32_t_uint32_t_GetRegion(this);
}
void* labstor_unordered_map_uint32_t_uint32_t::GetBaseRegion() {
    return labstor_unordered_map_uint32_t_uint32_t_GetBaseRegion(this);
}
uint32_t labstor_unordered_map_uint32_t_uint32_t::GetNumBuckets() {
    return labstor_unordered_map_uint32_t_uint32_t_GetNumBuckets(this);
}
uint32_t labstor_unordered_map_uint32_t_uint32_t::GetOverflow() {
    return labstor_unordered_map_uint32_t_uint32_t_GetOverflow(this);
}
void labstor_unordered_map_uint32_t_uint32_t::Init(void *base_region, void *region, uint32_t region_size, uint32_t max_collisions) {
    labstor_unordered_map_uint32_t_uint32_t_Init(this, base_region, region, region_size, max_collisions);
}
void labstor_unordered_map_uint32_t_uint32_t::Attach(void *base_region, void *region) {
    labstor_unordered_map_uint32_t_uint32_t_Attach(this, base_region, region);
}
int labstor_unordered_map_uint32_t_uint32_t::Set(struct labstor_uint32_t_uint32_t_bucket &bucket) {
    return labstor_unordered_map_uint32_t_uint32_t_Set(this, &bucket);
}
int labstor_unordered_map_uint32_t_uint32_t::Find(uint32_t key, uint32_t &value) {
    return labstor_unordered_map_uint32_t_uint32_t_Find(this, key, &value);
}
int labstor_unordered_map_uint32_t_uint32_t::Remove(uint32_t key) {
    return labstor_unordered_map_uint32_t_uint32_t_Remove(this, key);
}

#endif

#endif //LABSTOR_UNORDERED_MAP_uint32_t_uint32_t