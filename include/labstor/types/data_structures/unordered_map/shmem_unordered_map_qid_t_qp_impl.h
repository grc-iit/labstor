
//qid_t: The key's real type
//qid_t: The semantic name of type S
//qid_t: How the key is stored in the bucket
//labstor::ipc::queue_pair: The value's type
//qp: The semantic name of type T
//labstor_qid_t_qp_map_bucket: The semantic name of the bucket type
//labstor_qid_t_qp_map_bucket_hash(labstor::ipc::queue_pair key, void *region)
//labstor_qid_t_qp_map_bucket_GetAtomicKey()
//labstor_qid_t_qp_map_bucket_GetAtomicKeyRef()
//labstor_qid_t_qp_map_bucket_GetKey
//labstor_qid_t_qp_map_bucket_NullKey
//labstor_qid_t_qp_map_bucket_KeyCompare

#ifndef LABSTOR_UNORDERED_MAP_qid_t_qp
#define LABSTOR_UNORDERED_MAP_qid_t_qp

#include <labstor/kernel/types/data_structures/shmem_array_labstor_qid_t_qp_map_bucket.h>

struct labstor_unordered_map_qid_t_qp {
    struct labstor_array_labstor_qid_t_qp_map_bucket buckets_;
    struct labstor_array_labstor_qid_t_qp_map_bucket overflow_;
};

static inline bool labstor_unordered_map_qid_t_qp_AtomicSetKeyValue(struct labstor_array_labstor_qid_t_qp_map_bucket *arr, int i, struct labstor_qid_t_qp_map_bucket *bucket);
static inline bool labstor_unordered_map_qid_t_qp_AtomicGetValueByKey(struct labstor_array_labstor_qid_t_qp_map_bucket *arr, int i, qid_t key, labstor::ipc::queue_pair *value);
static inline bool labstor_unordered_map_qid_t_qp_AtomicNullifyKey(struct labstor_array_labstor_qid_t_qp_map_bucket *arr, int i, qid_t key);

static inline uint32_t labstor_unordered_map_qid_t_qp_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_labstor_qid_t_qp_map_bucket_GetSize_global(num_buckets) + labstor_array_labstor_qid_t_qp_map_bucket_GetSize_global(max_collisions);
}

static inline inline uint32_t labstor_unordered_map_qid_t_qp_GetSize(struct labstor_unordered_map_qid_t_qp *map) {
    return labstor_array_labstor_qid_t_qp_map_bucket_GetSize(&map->buckets_) + labstor_array_labstor_qid_t_qp_map_bucket_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_qid_t_qp_GetRegion(struct labstor_unordered_map_qid_t_qp *map) {
    return labstor_array_labstor_qid_t_qp_map_bucket_GetRegion(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_qid_t_qp_GetNumBuckets(struct labstor_unordered_map_qid_t_qp *map) {
    return labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_qid_t_qp_GetOverflow(struct labstor_unordered_map_qid_t_qp *map) {
    return labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_qid_t_qp_Init(struct labstor_unordered_map_qid_t_qp *map, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    overflow_region_size = labstor_array_labstor_qid_t_qp_map_bucket_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_labstor_qid_t_qp_map_bucket_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_labstor_qid_t_qp_map_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_qid_t_qp_map_bucket_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->buckets_); ++i) {
        *labstor_qid_t_qp_map_bucket_GetAtomicKeyRef(labstor_array_labstor_qid_t_qp_map_bucket_GetPtr(&map->buckets_, i)) = labstor_qid_t_qp_map_bucket_NullKey();
    }
    for(i = 0; i < labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->overflow_); ++i) {
        *labstor_qid_t_qp_map_bucket_GetAtomicKeyRef(labstor_array_labstor_qid_t_qp_map_bucket_GetPtr(&map->overflow_, i)) = labstor_qid_t_qp_map_bucket_NullKey();
    }
}

static inline void labstor_unordered_map_qid_t_qp_Attach(struct labstor_unordered_map_qid_t_qp *map, void *region) {
    labstor_array_labstor_qid_t_qp_map_bucket_Attach(&map->buckets_, region);
    region = labstor_array_labstor_qid_t_qp_map_bucket_GetNextSection(&map->buckets_);
    labstor_array_labstor_qid_t_qp_map_bucket_Attach(&map->overflow_, region);
}

static inline bool labstor_unordered_map_qid_t_qp_Set(struct labstor_unordered_map_qid_t_qp *map, struct labstor_qid_t_qp_map_bucket *bucket) {
    void *bucket_region;
    uint32_t b;
    int i;

    bucket_region = labstor_array_labstor_qid_t_qp_map_bucket_GetRegion(&map->buckets_);
    b = labstor_qid_t_qp_map_bucket_hash(labstor_qid_t_qp_map_bucket_GetKey(bucket, bucket_region), bucket_region) % labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->buckets_);
    if(labstor_unordered_map_qid_t_qp_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_qid_t_qp_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline bool labstor_unordered_map_qid_t_qp_Find(struct labstor_unordered_map_qid_t_qp *map, qid_t key, labstor::ipc::queue_pair *value) {
    uint32_t b = labstor_qid_t_qp_map_bucket_hash(key, labstor_array_labstor_qid_t_qp_map_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_qid_t_qp_AtomicGetValueByKey(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_qid_t_qp_AtomicGetValueByKey(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline bool labstor_unordered_map_qid_t_qp_Remove(struct labstor_unordered_map_qid_t_qp *map, qid_t key) {
    int32_t b = labstor_qid_t_qp_map_bucket_hash(key, labstor_array_labstor_qid_t_qp_map_bucket_GetRegion(&map->buckets_)) % labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_qid_t_qp_AtomicNullifyKey(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_labstor_qid_t_qp_map_bucket_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_qid_t_qp_AtomicNullifyKey(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline bool labstor_unordered_map_qid_t_qp_AtomicSetKeyValue(struct labstor_array_labstor_qid_t_qp_map_bucket *arr, int i, struct labstor_qid_t_qp_map_bucket *bucket) {
    qid_t null = labstor_qid_t_qp_map_bucket_NullKey();
    if(__atomic_compare_exchange_n(
            labstor_qid_t_qp_map_bucket_GetAtomicKeyRef(labstor_array_labstor_qid_t_qp_map_bucket_GetPtr(arr, i)),
            &null,
            labstor_qid_t_qp_map_bucket_GetAtomicKey(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_labstor_qid_t_qp_map_bucket_GetPtr(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline bool labstor_unordered_map_qid_t_qp_AtomicGetValueByKey(struct labstor_array_labstor_qid_t_qp_map_bucket *arr, int i, qid_t key, labstor::ipc::queue_pair *value) {
    struct labstor_qid_t_qp_map_bucket tmp;
    do {
        tmp = labstor_array_labstor_qid_t_qp_map_bucket_Get(arr, i);
        if (labstor_qid_t_qp_map_bucket_KeyCompare(
                labstor_qid_t_qp_map_bucket_GetKey(&tmp, labstor_array_labstor_qid_t_qp_map_bucket_GetRegion(arr)),
                key)
        ) { return false; }
        *value = labstor_qid_t_qp_map_bucket_GetValue(&tmp, labstor_array_labstor_qid_t_qp_map_bucket_GetRegion(arr));
    } while(!__atomic_compare_exchange_n(
            labstor_qid_t_qp_map_bucket_GetAtomicKeyRef(labstor_array_labstor_qid_t_qp_map_bucket_GetPtr(arr, i)),
            labstor_qid_t_qp_map_bucket_GetAtomicKeyRef(&tmp),
            labstor_qid_t_qp_map_bucket_GetAtomicKey(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline bool labstor_unordered_map_qid_t_qp_AtomicNullifyKey(struct labstor_array_labstor_qid_t_qp_map_bucket *arr, int i, qid_t key) {
    struct labstor_qid_t_qp_map_bucket tmp;
    do {
        tmp = labstor_array_labstor_qid_t_qp_map_bucket_Get(arr, i);
        if (labstor_qid_t_qp_map_bucket_KeyCompare(
                labstor_qid_t_qp_map_bucket_GetKey(&tmp, labstor_array_labstor_qid_t_qp_map_bucket_GetRegion(arr)),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            labstor_qid_t_qp_map_bucket_GetAtomicKeyRef(labstor_array_labstor_qid_t_qp_map_bucket_GetPtr(arr, i)),
            labstor_qid_t_qp_map_bucket_GetAtomicKeyRef(&tmp),
            labstor_qid_t_qp_map_bucket_NullKey(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#ifdef __cplusplus

namespace labstor::ipc {

class unordered_map_qid_t_qp; : protected labstor_unordered_map_qid_t_qp, public shmem_type {
public:
    static uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions) {
        return labstor_unordered_map_qid_t_qp_GetSize_global(num_buckets, max_collisions);
    }
    inline uint32_t GetSize() {
        return labstor_unordered_map_qid_t_qp_GetSize(this);
    }
    inline void* GetRegion() {
        return labstor_unordered_map_qid_t_qp_GetRegion(this);
    }
    inline uint32_t GetNumBuckets() {
        return labstor_unordered_map_qid_t_qp_GetNumBuckets(this);
    }
    inline uint32_t GetOverflow() {
        return labstor_unordered_map_qid_t_qp_GetOverflow(this);
    }

    inline void Init(void *region, uint32_t region_size, uint32_t max_collisions) {
        labstor_unordered_map_qid_t_qp_Init(this, region, region_size, max_collisions);
    }
    inline void Attach(void *region) {
        labstor_unordered_map_qid_t_qp_Attach(this, region);
    }

    inline bool Set(struct labstor_qid_t_qp_map_bucket &bucket) {
        return labstor_unordered_map_qid_t_qp_Set(this, &bucket);
    }

    inline bool Find(qid_t key, labstor::ipc::queue_pair &value) {
        return labstor_unordered_map_qid_t_qp_Find(this, key, &value);
    }

    inline bool Remove(qid_t key) {
        return labstor_unordered_map_qid_t_qp_Remove(this, key);
    }

    inline T operator [](qid_t key) {
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

#endif //LABSTOR_UNORDERED_MAP_qid_t_qp