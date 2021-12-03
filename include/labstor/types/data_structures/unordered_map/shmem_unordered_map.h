
//{S}: The key's real type
//{S_NAME}: The semantic name of type S
//{S_ATOMIC}: How the key is stored in the bucket
//{T}: The value's type
//{T_NAME}: The semantic name of type T
//{BUCKET_T_NAME}: The semantic name of the bucket type
//{KeyHash}({T} key, void *region)
//{GetAtomicKey}()
//{GetAtomicKeyRef}()
//{GetKey}
//{NullKey}
//{KeyCompare}

#ifndef LABSTOR_UNORDERED_MAP_{S_NAME}_{T_NAME}
#define LABSTOR_UNORDERED_MAP_{S_NAME}_{T_NAME}

#include <labstor/kernel/types/data_structures/shmem_array_{BUCKET_T_NAME}.h>

struct labstor_unordered_map_{S_NAME}_{T_NAME} {
    struct labstor_array_{BUCKET_T_NAME} buckets_;
    struct labstor_array_{BUCKET_T_NAME} overflow_;
};

static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicSetKeyValue(struct labstor_array_{BUCKET_T_NAME} *arr, int i, struct {BUCKET_T_NAME} *bucket);
static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicGetValueByKey(struct labstor_array_{BUCKET_T_NAME} *arr, int i, {S} key, {T} *value);
static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicNullifyKey(struct labstor_array_{BUCKET_T_NAME} *arr, int i, {S} key);

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_{BUCKET_T_NAME}_GetSize_global(num_buckets) + labstor_array_{BUCKET_T_NAME}_GetSize_global(max_collisions);
}

static inline inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_array_{BUCKET_T_NAME}_GetSize(&map->buckets_) + labstor_array_{BUCKET_T_NAME}_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_{S_NAME}_{T_NAME}_GetRegion(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_array_{BUCKET_T_NAME}_GetRegion(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetNumBuckets(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetOverflow(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_array_{BUCKET_T_NAME}_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_{S_NAME}_{T_NAME}_Init(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    overflow_region_size = labstor_array_{BUCKET_T_NAME}_GetSize_global(max_collisions);
    bucket_region_size = region_size - overflow_region_size;
    labstor_array_{BUCKET_T_NAME}_Init(&map->buckets_, region, bucket_region_size, 0);
    region = labstor_array_{BUCKET_T_NAME}_GetNextSection(&map->buckets_);
    labstor_array_{BUCKET_T_NAME}_Init(&map->overflow_, region, overflow_region_size, 0);

    //Initialize buckets
    for(i = 0; i < labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_); ++i) {
        *{GetAtomicKeyRef}(labstor_array_{BUCKET_T_NAME}_GetPtr(&map->buckets_, i)) = {NullKey}();
    }
    for(i = 0; i < labstor_array_{BUCKET_T_NAME}_GetLength(&map->overflow_); ++i) {
        *{GetAtomicKeyRef}(labstor_array_{BUCKET_T_NAME}_GetPtr(&map->overflow_, i)) = {NullKey}();
    }
}

static inline void labstor_unordered_map_{S_NAME}_{T_NAME}_Attach(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, void *region) {
    labstor_array_{BUCKET_T_NAME}_Attach(&map->buckets_, region);
    region = labstor_array_{BUCKET_T_NAME}_GetNextSection(&map->buckets_);
    labstor_array_{BUCKET_T_NAME}_Attach(&map->overflow_, region);
}

static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_Set(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, struct {BUCKET_T_NAME} *bucket) {
    void *bucket_region;
    uint32_t b;
    int i;

    bucket_region = labstor_array_{BUCKET_T_NAME}_GetRegion(&map->buckets_);
    b = {KeyHash}({GetKey}(bucket, bucket_region), bucket_region) % labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_);
    if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicSetKeyValue(&map->buckets_, b, bucket)) {
        return true;
    }
    for(i = 0; i < labstor_array_{BUCKET_T_NAME}_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicSetKeyValue(&map->overflow_, i, bucket)) {
            return true;
        }
    }
    return false;
}

static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_Find(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, {S} key, {T} *value) {
    uint32_t b = {KeyHash}(key, labstor_array_{BUCKET_T_NAME}_GetRegion(&map->buckets_)) % labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicGetValueByKey(&map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_{BUCKET_T_NAME}_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicGetValueByKey(&map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_Remove(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, {S} key) {
    int32_t b = {KeyHash}(key, labstor_array_{BUCKET_T_NAME}_GetRegion(&map->buckets_)) % labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicNullifyKey(&map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_{BUCKET_T_NAME}_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicNullifyKey(&map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicSetKeyValue(struct labstor_array_{BUCKET_T_NAME} *arr, int i, struct {BUCKET_T_NAME} *bucket) {
    {S_ATOMIC} null = {NullKey}();
    if(__atomic_compare_exchange_n(
            {GetAtomicKeyRef}(labstor_array_{BUCKET_T_NAME}_GetPtr(arr, i)),
            &null,
            {GetAtomicKey}(bucket),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        *labstor_array_{BUCKET_T_NAME}_GetPtr(arr, i) = *bucket;
        return true;
    }
    return false;
}

static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicGetValueByKey(struct labstor_array_{BUCKET_T_NAME} *arr, int i, {S} key, {T} *value) {
    struct {BUCKET_T_NAME} tmp;
    do {
        tmp = labstor_array_{BUCKET_T_NAME}_Get(arr, i);
        if ({KeyCompare}(
                {GetKey}(&tmp, labstor_array_{BUCKET_T_NAME}_GetRegion(arr)),
                key)
        ) { return false; }
        *value = {GetValue}(&tmp, labstor_array_{BUCKET_T_NAME}_GetRegion(arr));
    } while(!__atomic_compare_exchange_n(
            {GetAtomicKeyRef}(labstor_array_{BUCKET_T_NAME}_GetPtr(arr, i)),
            {GetAtomicKeyRef}(&tmp),
            {GetAtomicKey}(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicNullifyKey(struct labstor_array_{BUCKET_T_NAME} *arr, int i, {S} key) {
    struct {BUCKET_T_NAME} tmp;
    do {
        tmp = labstor_array_{BUCKET_T_NAME}_Get(arr, i);
        if ({KeyCompare}(
                {GetKey}(&tmp, labstor_array_{BUCKET_T_NAME}_GetRegion(arr)),
                key)
        ) { return false; }
    } while(!__atomic_compare_exchange_n(
            {GetAtomicKeyRef}(labstor_array_{BUCKET_T_NAME}_GetPtr(arr, i)),
            {GetAtomicKeyRef}(&tmp),
            {NullKey}(),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    return true;
}

#ifdef __cplusplus

namespace labstor::ipc {

class unordered_map_{S_NAME}_{T_NAME}; : protected labstor_unordered_map_{S_NAME}_{T_NAME}, public shmem_type {
public:
    static uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions) {
        return labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize_global(num_buckets, max_collisions);
    }
    inline uint32_t GetSize() {
        return labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize(this);
    }
    inline void* GetRegion() {
        return labstor_unordered_map_{S_NAME}_{T_NAME}_GetRegion(this);
    }
    inline uint32_t GetNumBuckets() {
        return labstor_unordered_map_{S_NAME}_{T_NAME}_GetNumBuckets(this);
    }
    inline uint32_t GetOverflow() {
        return labstor_unordered_map_{S_NAME}_{T_NAME}_GetOverflow(this);
    }

    inline void Init(void *region, uint32_t region_size, uint32_t max_collisions) {
        labstor_unordered_map_{S_NAME}_{T_NAME}_Init(this, region, region_size, max_collisions);
    }
    inline void Attach(void *region) {
        labstor_unordered_map_{S_NAME}_{T_NAME}_Attach(this, region);
    }

    inline bool Set(struct {BUCKET_T_NAME} &bucket) {
        return labstor_unordered_map_{S_NAME}_{T_NAME}_Set(this, &bucket);
    }

    inline bool Find({S} key, {T} &value) {
        return labstor_unordered_map_{S_NAME}_{T_NAME}_Find(this, key, &value);
    }

    inline bool Remove({S} key) {
        return labstor_unordered_map_{S_NAME}_{T_NAME}_Remove(this, key);
    }

    inline T operator []({S} key) {
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

#endif //LABSTOR_UNORDERED_MAP_{S_NAME}_{T_NAME}