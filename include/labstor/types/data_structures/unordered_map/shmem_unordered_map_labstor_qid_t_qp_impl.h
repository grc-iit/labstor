
//labstor_qid_t: The key's real type
//labstor_qid_t: The semantic name of type S
//labstor_qid_t: How the key is stored in the bucket
//labstor::ipc::queue_pair*: The value's type
//qp: The semantic name of type T
//labstor_labstor_qid_t_qp_bucket: The semantic name of the bucket type
//labstor_labstor_qid_t_qp_bucket_hash(labstor::ipc::queue_pair* key, void *region)
//labstor_labstor_qid_t_qp_bucket_GetAtomicKey()
//labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef()
//labstor_labstor_qid_t_qp_bucket_GetKey
//labstor_labstor_qid_t_qp_bucket_NullKey
//labstor_labstor_qid_t_qp_bucket_KeyCompare

#ifndef LABSTOR_UNORDERED_MAP_labstor_qid_t_qp
#define LABSTOR_UNORDERED_MAP_labstor_qid_t_qp

#include <labstor/types/data_structures/array/shmem_array_labstor_labstor_qid_t_qp_bucket.h>

#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
#endif

#ifdef __cplusplus
struct labstor_unordered_map_labstor_qid_t_qp : public labstor::shmem_type {
#else
struct labstor_unordered_map_labstor_qid_t_qp {
#endif
    struct labstor_array_labstor_labstor_qid_t_qp_bucket buckets_;
    struct labstor_array_labstor_labstor_qid_t_qp_bucket overflow_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline uint32_t GetNumBuckets();
    inline uint32_t GetOverflow();
    inline void Init(void *region, uint32_t region_size, uint32_t max_collisions);
    inline void Attach(void *region);
    inline int Set(struct labstor_labstor_qid_t_qp_bucket &bucket);
    inline int Find(labstor_qid_t key, labstor::ipc::queue_pair* &value);
    inline int Remove(labstor_qid_t key);
    inline labstor::ipc::queue_pair* operator [](labstor_qid_t key) {
        labstor::ipc::queue_pair* value;
        if(Find(key, value)) {
            return value;
        }
        throw labstor::INVALID_UNORDERED_MAP_KEY.format();
    }
#endif
};

static inline int labstor_unordered_map_labstor_qid_t_qp_AtomicSetKeyValue(struct labstor_array_labstor_labstor_qid_t_qp_bucket *arr, int i, struct labstor_labstor_qid_t_qp_bucket *bucket);
static inline int labstor_unordered_map_labstor_qid_t_qp_AtomicGetValueByKey(struct labstor_array_labstor_labstor_qid_t_qp_bucket *arr, int i, labstor_qid_t key, labstor::ipc::queue_pair* *value);
static inline int labstor_unordered_map_labstor_qid_t_qp_AtomicNullifyKey(struct labstor_array_labstor_labstor_qid_t_qp_bucket *arr, int i, labstor_qid_t key);

static inline uint32_t labstor_unordered_map_labstor_qid_t_qp_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_labstor_labstor_qid_t_qp_bucket_GetSize_global(num_buckets) + labstor_array_labstor_labstor_qid_t_qp_bucket_GetSize_global(max_collisions);
}

static inline uint32_t labstor_unordered_map_labstor_qid_t_qp_GetSize(struct labstor_unordered_map_labstor_qid_t_qp *map) {
    return labstor_array_labstor_labstor_qid_t_qp_bucket_GetSize(&map->buckets_) + labstor_array_labstor_labstor_qid_t_qp_bucket_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_labstor_qid_t_qp_GetRegion(struct labstor_unordered_map_labstor_qid_t_qp *map) {
    return labstor_array_labstor_labstor_qid_t_qp_bucket_GetRegion(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_labstor_qid_t_qp_GetNumBuckets(struct labstor_unordered_map_labstor_qid_t_qp *map) {
    return labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_labstor_qid_t_qp_GetOverflow(struct labstor_unordered_map_labstor_qid_t_qp *map) {
    return labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_labstor_qid_t_qp_Init(struct labstor_unordered_map_labstor_qid_t_qp *map, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    overflow_region_size = labstor_array_labstor_labstor_qid_t_qp_bucket_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_labstor_labstor_qid_t_qp_bucket_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_labstor_labstor_qid_t_qp_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_labstor_qid_t_qp_bucket_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->buckets_); ++i) {
        *labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef(labstor_array_labstor_labstor_qid_t_qp_bucket_GetPtr(&map->buckets_, i)) = labstor_labstor_qid_t_qp_bucket_NullKey();
    }
    for(i = 0; i < labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->overflow_); ++i) {
        *labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef(labstor_array_labstor_labstor_qid_t_qp_bucket_GetPtr(&map->overflow_, i)) = labstor_labstor_qid_t_qp_bucket_NullKey();
    }
}

static inline void labstor_unordered_map_labstor_qid_t_qp_Attach(struct labstor_unordered_map_labstor_qid_t_qp *map, void *region) {
    labstor_array_labstor_labstor_qid_t_qp_bucket_Attach(&map->buckets_, region);
    region = labstor_array_labstor_labstor_qid_t_qp_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_labstor_qid_t_qp_bucket_Attach(&map->overflow_, region);
}

static inline int labstor_unordered_map_labstor_qid_t_qp_Set(struct labstor_unordered_map_labstor_qid_t_qp *map, struct labstor_labstor_qid_t_qp_bucket *bucket) {
    void *bucket_region;
    uint32_t b;
    int i;

    bucket_region = labstor_array_labstor_labstor_qid_t_qp_bucket_GetRegion(&map->buckets_);
    b = labstor_labstor_qid_t_qp_bucket_hash(labstor_labstor_qid_t_qp_bucket_GetKey(bucket, bucket_region), bucket_region) % labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->buckets_);
    if(labstor_unordered_map_labstor_qid_t_qp_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_labstor_qid_t_qp_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline int labstor_unordered_map_labstor_qid_t_qp_Find(struct labstor_unordered_map_labstor_qid_t_qp *map, labstor_qid_t key, labstor::ipc::queue_pair* *value) {
    uint32_t b = labstor_labstor_qid_t_qp_bucket_hash(key, labstor_array_labstor_labstor_qid_t_qp_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_labstor_qid_t_qp_AtomicGetValueByKey(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_labstor_qid_t_qp_AtomicGetValueByKey(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline int labstor_unordered_map_labstor_qid_t_qp_Remove(struct labstor_unordered_map_labstor_qid_t_qp *map, labstor_qid_t key) {
    int32_t b = labstor_labstor_qid_t_qp_bucket_hash(key, labstor_array_labstor_labstor_qid_t_qp_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_labstor_qid_t_qp_AtomicNullifyKey(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_labstor_labstor_qid_t_qp_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_labstor_qid_t_qp_AtomicNullifyKey(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline int labstor_unordered_map_labstor_qid_t_qp_AtomicSetKeyValue(struct labstor_array_labstor_labstor_qid_t_qp_bucket *arr, int i, struct labstor_labstor_qid_t_qp_bucket *bucket) {
    labstor_qid_t null = labstor_labstor_qid_t_qp_bucket_NullKey();
    if(__atomic_compare_exchange_n(
            labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef(labstor_array_labstor_labstor_qid_t_qp_bucket_GetPtr(arr, i)),
            &null,
            labstor_labstor_qid_t_qp_bucket_GetAtomicKey(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_labstor_labstor_qid_t_qp_bucket_GetPtr(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline int labstor_unordered_map_labstor_qid_t_qp_AtomicGetValueByKey(struct labstor_array_labstor_labstor_qid_t_qp_bucket *arr, int i, labstor_qid_t key, labstor::ipc::queue_pair* *value) {
    struct labstor_labstor_qid_t_qp_bucket tmp;
    do {
        tmp = labstor_array_labstor_labstor_qid_t_qp_bucket_Get(arr, i);
        if (!labstor_labstor_qid_t_qp_bucket_KeyCompare(
                labstor_labstor_qid_t_qp_bucket_GetKey(&tmp, labstor_array_labstor_labstor_qid_t_qp_bucket_GetRegion(arr)),
                key)
        ) { return false; }
        *value = labstor_labstor_qid_t_qp_bucket_GetValue(&tmp, labstor_array_labstor_labstor_qid_t_qp_bucket_GetRegion(arr));
    } while(!__atomic_compare_exchange_n(
            labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef(labstor_array_labstor_labstor_qid_t_qp_bucket_GetPtr(arr, i)),
            labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef(&tmp),
            labstor_labstor_qid_t_qp_bucket_GetAtomicKey(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline int labstor_unordered_map_labstor_qid_t_qp_AtomicNullifyKey(struct labstor_array_labstor_labstor_qid_t_qp_bucket *arr, int i, labstor_qid_t key) {
    struct labstor_labstor_qid_t_qp_bucket tmp;
    do {
        tmp = labstor_array_labstor_labstor_qid_t_qp_bucket_Get(arr, i);
        if (!labstor_labstor_qid_t_qp_bucket_KeyCompare(
                labstor_labstor_qid_t_qp_bucket_GetKey(&tmp, labstor_array_labstor_labstor_qid_t_qp_bucket_GetRegion(arr)),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef(labstor_array_labstor_labstor_qid_t_qp_bucket_GetPtr(arr, i)),
            labstor_labstor_qid_t_qp_bucket_GetAtomicKeyRef(&tmp),
            labstor_labstor_qid_t_qp_bucket_NullKey(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#ifdef __cplusplus

namespace labstor::ipc {
    typedef labstor_unordered_map_labstor_qid_t_qp unordered_map_labstor_qid_t_qp;
}
uint32_t labstor_unordered_map_labstor_qid_t_qp::GetSize(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_unordered_map_labstor_qid_t_qp_GetSize_global(num_buckets, max_collisions);
}
uint32_t labstor_unordered_map_labstor_qid_t_qp::GetSize() {
    return labstor_unordered_map_labstor_qid_t_qp_GetSize(this);
}
void* labstor_unordered_map_labstor_qid_t_qp::GetRegion() {
    return labstor_unordered_map_labstor_qid_t_qp_GetRegion(this);
}
uint32_t labstor_unordered_map_labstor_qid_t_qp::GetNumBuckets() {
    return labstor_unordered_map_labstor_qid_t_qp_GetNumBuckets(this);
}
uint32_t labstor_unordered_map_labstor_qid_t_qp::GetOverflow() {
    return labstor_unordered_map_labstor_qid_t_qp_GetOverflow(this);
}
void labstor_unordered_map_labstor_qid_t_qp::Init(void *region, uint32_t region_size, uint32_t max_collisions) {
    labstor_unordered_map_labstor_qid_t_qp_Init(this, region, region_size, max_collisions);
}
void labstor_unordered_map_labstor_qid_t_qp::Attach(void *region) {
    labstor_unordered_map_labstor_qid_t_qp_Attach(this, region);
}
int labstor_unordered_map_labstor_qid_t_qp::Set(struct labstor_labstor_qid_t_qp_bucket &bucket) {
    return labstor_unordered_map_labstor_qid_t_qp_Set(this, &bucket);
}
int labstor_unordered_map_labstor_qid_t_qp::Find(labstor_qid_t key, labstor::ipc::queue_pair* &value) {
    return labstor_unordered_map_labstor_qid_t_qp_Find(this, key, &value);
}
int labstor_unordered_map_labstor_qid_t_qp::Remove(labstor_qid_t key) {
    return labstor_unordered_map_labstor_qid_t_qp_Remove(this, key);
}

#endif

#endif //LABSTOR_UNORDERED_MAP_labstor_qid_t_qp