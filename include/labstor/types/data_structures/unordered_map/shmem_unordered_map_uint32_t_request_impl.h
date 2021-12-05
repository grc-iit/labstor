
//uint32_t: The key's real type
//uint32_t: The semantic name of type S
//uint32_t: How the key is stored in the bucket
//struct labstor_request*: The value's type
//request: The semantic name of type T
//labstor_request_map_bucket: The semantic name of the bucket type
//labstor_request_map_bucket_hash(struct labstor_request* key, void *region)
//labstor_request_map_bucket_GetAtomicKey()
//labstor_request_map_bucket_GetAtomicKeyRef()
//labstor_request_map_bucket_GetKey
//labstor_request_map_bucket_NullKey
//labstor_request_map_bucket_KeyCompare

#ifndef LABSTOR_UNORDERED_MAP_uint32_t_request
#define LABSTOR_UNORDERED_MAP_uint32_t_request

#include <labstor/types/data_structures/array/shmem_array_labstor_request_map_bucket.h>

#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
#endif

#ifdef __cplusplus
struct labstor_unordered_map_uint32_t_request : public labstor::shmem_type {
#else
struct labstor_unordered_map_uint32_t_request {
#endif
    struct labstor_array_labstor_request_map_bucket buckets_;
    struct labstor_array_labstor_request_map_bucket overflow_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline uint32_t GetNumBuckets();
    inline uint32_t GetOverflow();
    inline void Init(void *region, uint32_t region_size, uint32_t max_collisions);
    inline void Attach(void *region);
    inline int Set(struct labstor_request_map_bucket &bucket);
    inline int Find(uint32_t key, struct labstor_request* &value);
    inline int Remove(uint32_t key);
    inline struct labstor_request* operator [](uint32_t key) {
        struct labstor_request* value;
        if(Find(key, value)) {
            return value;
        }
        throw labstor::INVALID_UNORDERED_MAP_KEY.format();
    }
#endif
};

static inline int labstor_unordered_map_uint32_t_request_AtomicSetKeyValue(struct labstor_array_labstor_request_map_bucket *arr, int i, struct labstor_request_map_bucket *bucket);
static inline int labstor_unordered_map_uint32_t_request_AtomicGetValueByKey(struct labstor_array_labstor_request_map_bucket *arr, int i, uint32_t key, struct labstor_request* *value);
static inline int labstor_unordered_map_uint32_t_request_AtomicNullifyKey(struct labstor_array_labstor_request_map_bucket *arr, int i, uint32_t key);

static inline uint32_t labstor_unordered_map_uint32_t_request_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_labstor_request_map_bucket_GetSize_global(num_buckets) + labstor_array_labstor_request_map_bucket_GetSize_global(max_collisions);
}

static inline uint32_t labstor_unordered_map_uint32_t_request_GetSize(struct labstor_unordered_map_uint32_t_request *map) {
    return labstor_array_labstor_request_map_bucket_GetSize(&map->buckets_) + labstor_array_labstor_request_map_bucket_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_uint32_t_request_GetRegion(struct labstor_unordered_map_uint32_t_request *map) {
    return labstor_array_labstor_request_map_bucket_GetRegion(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_uint32_t_request_GetNumBuckets(struct labstor_unordered_map_uint32_t_request *map) {
    return labstor_array_labstor_request_map_bucket_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_uint32_t_request_GetOverflow(struct labstor_unordered_map_uint32_t_request *map) {
    return labstor_array_labstor_request_map_bucket_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_uint32_t_request_Init(struct labstor_unordered_map_uint32_t_request *map, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    overflow_region_size = labstor_array_labstor_request_map_bucket_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_labstor_request_map_bucket_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_labstor_request_map_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_request_map_bucket_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_labstor_request_map_bucket_GetLength(&map->buckets_); ++i) {
        *labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_labstor_request_map_bucket_GetPtr(&map->buckets_, i)) = labstor_request_map_bucket_NullKey();
    }
    for(i = 0; i < labstor_array_labstor_request_map_bucket_GetLength(&map->overflow_); ++i) {
        *labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_labstor_request_map_bucket_GetPtr(&map->overflow_, i)) = labstor_request_map_bucket_NullKey();
    }
}

static inline void labstor_unordered_map_uint32_t_request_Attach(struct labstor_unordered_map_uint32_t_request *map, void *region) {
    labstor_array_labstor_request_map_bucket_Attach(&map->buckets_, region);
    region = labstor_array_labstor_request_map_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_request_map_bucket_Attach(&map->overflow_, region);
}

static inline int labstor_unordered_map_uint32_t_request_Set(struct labstor_unordered_map_uint32_t_request *map, struct labstor_request_map_bucket *bucket) {
    void *bucket_region;
    uint32_t b;
    int i;

    bucket_region = labstor_array_labstor_request_map_bucket_GetRegion(&map->buckets_);
    b = labstor_request_map_bucket_hash(labstor_request_map_bucket_GetKey(bucket, bucket_region), bucket_region) % labstor_array_labstor_request_map_bucket_GetLength(&map->buckets_);
    if(labstor_unordered_map_uint32_t_request_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_labstor_request_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_uint32_t_request_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline int labstor_unordered_map_uint32_t_request_Find(struct labstor_unordered_map_uint32_t_request *map, uint32_t key, struct labstor_request* *value) {
    uint32_t b = labstor_request_map_bucket_hash(key, labstor_array_labstor_request_map_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_request_map_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_uint32_t_request_AtomicGetValueByKey(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_labstor_request_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_uint32_t_request_AtomicGetValueByKey(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline int labstor_unordered_map_uint32_t_request_Remove(struct labstor_unordered_map_uint32_t_request *map, uint32_t key) {
    int32_t b = labstor_request_map_bucket_hash(key, labstor_array_labstor_request_map_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_request_map_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_uint32_t_request_AtomicNullifyKey(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_labstor_request_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_uint32_t_request_AtomicNullifyKey(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline int labstor_unordered_map_uint32_t_request_AtomicSetKeyValue(struct labstor_array_labstor_request_map_bucket *arr, int i, struct labstor_request_map_bucket *bucket) {
    uint32_t null = labstor_request_map_bucket_NullKey();
    if(__atomic_compare_exchange_n(
            labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_labstor_request_map_bucket_GetPtr(arr, i)),
            &null,
            labstor_request_map_bucket_GetAtomicKey(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_labstor_request_map_bucket_GetPtr(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline int labstor_unordered_map_uint32_t_request_AtomicGetValueByKey(struct labstor_array_labstor_request_map_bucket *arr, int i, uint32_t key, struct labstor_request* *value) {
    struct labstor_request_map_bucket tmp;
    do {
        tmp = labstor_array_labstor_request_map_bucket_Get(arr, i);
        if (!labstor_request_map_bucket_KeyCompare(
                labstor_request_map_bucket_GetKey(&tmp, labstor_array_labstor_request_map_bucket_GetRegion(arr)),
                key)
        ) { return false; }
        *value = labstor_request_map_bucket_GetValue(&tmp, labstor_array_labstor_request_map_bucket_GetRegion(arr));
    } while(!__atomic_compare_exchange_n(
            labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_labstor_request_map_bucket_GetPtr(arr, i)),
            labstor_request_map_bucket_GetAtomicKeyRef(&tmp),
            labstor_request_map_bucket_GetAtomicKey(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline int labstor_unordered_map_uint32_t_request_AtomicNullifyKey(struct labstor_array_labstor_request_map_bucket *arr, int i, uint32_t key) {
    struct labstor_request_map_bucket tmp;
    do {
        tmp = labstor_array_labstor_request_map_bucket_Get(arr, i);
        if (!labstor_request_map_bucket_KeyCompare(
                labstor_request_map_bucket_GetKey(&tmp, labstor_array_labstor_request_map_bucket_GetRegion(arr)),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            labstor_request_map_bucket_GetAtomicKeyRef(labstor_array_labstor_request_map_bucket_GetPtr(arr, i)),
            labstor_request_map_bucket_GetAtomicKeyRef(&tmp),
            labstor_request_map_bucket_NullKey(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#ifdef __cplusplus

namespace labstor::ipc {
    typedef labstor_unordered_map_uint32_t_request unordered_map_uint32_t_request;
}

uint32_t labstor::ipc::unordered_map_uint32_t_request::GetSize(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_unordered_map_uint32_t_request_GetSize_global(num_buckets, max_collisions);
}
uint32_t labstor::ipc::unordered_map_uint32_t_request::GetSize() {
    return labstor_unordered_map_uint32_t_request_GetSize(this);
}
void* labstor::ipc::unordered_map_uint32_t_request::GetRegion() {
    return labstor_unordered_map_uint32_t_request_GetRegion(this);
}
uint32_t labstor::ipc::unordered_map_uint32_t_request::GetNumBuckets() {
    return labstor_unordered_map_uint32_t_request_GetNumBuckets(this);
}
uint32_t labstor::ipc::unordered_map_uint32_t_request::GetOverflow() {
    return labstor_unordered_map_uint32_t_request_GetOverflow(this);
}
void labstor::ipc::unordered_map_uint32_t_request::Init(void *region, uint32_t region_size, uint32_t max_collisions) {
    labstor_unordered_map_uint32_t_request_Init(this, region, region_size, max_collisions);
}
void labstor::ipc::unordered_map_uint32_t_request::Attach(void *region) {
    labstor_unordered_map_uint32_t_request_Attach(this, region);
}
int labstor::ipc::unordered_map_uint32_t_request::Set(struct labstor_request_map_bucket &bucket) {
    return labstor_unordered_map_uint32_t_request_Set(this, &bucket);
}
int labstor::ipc::unordered_map_uint32_t_request::Find(uint32_t key, struct labstor_request* &value) {
    return labstor_unordered_map_uint32_t_request_Find(this, key, &value);
}
int labstor::ipc::unordered_map_uint32_t_request::Remove(uint32_t key) {
    return labstor_unordered_map_uint32_t_request_Remove(this, key);
}

#endif

#endif //LABSTOR_UNORDERED_MAP_uint32_t_request