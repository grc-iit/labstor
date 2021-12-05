
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

#include <labstor/types/data_structures/array/shmem_array_{BUCKET_T_NAME}.h>

#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
#endif

#ifdef __cplusplus
struct labstor_unordered_map_{S_NAME}_{T_NAME} : public labstor::shmem_type {
#else
struct labstor_unordered_map_{S_NAME}_{T_NAME} {
#endif
    void *base_region_;
    struct labstor_array_{BUCKET_T_NAME} buckets_;
    struct labstor_array_{BUCKET_T_NAME} overflow_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t num_buckets, uint32_t max_collisions);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void* GetBaseRegion();
    inline uint32_t GetNumBuckets();
    inline uint32_t GetOverflow();
    inline void Init(void *base_region, void *region, uint32_t region_size, uint32_t max_collisions);
    inline void Attach(void *base_region, void *region);
    inline int Set(struct {BUCKET_T_NAME} &bucket);
    inline int Find({S} key, {T} &value);
    inline int Remove({S} key);
    inline {T} operator []({S} key) {
        {T} value;
        if(Find(key, value)) {
            return value;
        }
        throw labstor::INVALID_UNORDERED_MAP_KEY.format();
    }
#endif
};

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicSetKeyValue(struct labstor_array_{BUCKET_T_NAME} *arr, int i, struct {BUCKET_T_NAME} *bucket);
static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicGetValueByKey(void *base_region, struct labstor_array_{BUCKET_T_NAME} *arr, int i, {S} key, {T} *value);
static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicNullifyKey(void *base_region, struct labstor_array_{BUCKET_T_NAME} *arr, int i, {S} key);

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize_global(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_array_{BUCKET_T_NAME}_GetSize_global(num_buckets) + labstor_array_{BUCKET_T_NAME}_GetSize_global(max_collisions);
}

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_array_{BUCKET_T_NAME}_GetSize(&map->buckets_) + labstor_array_{BUCKET_T_NAME}_GetSize(&map->overflow_);
}

static inline void* labstor_unordered_map_{S_NAME}_{T_NAME}_GetRegion(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_array_{BUCKET_T_NAME}_GetRegion(&map->buckets_);
}

static inline void* labstor_unordered_map_{S_NAME}_{T_NAME}_GetBaseRegion(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return map->base_region_;
}

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetNumBuckets(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_);
}

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetOverflow(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_array_{BUCKET_T_NAME}_GetLength(&map->overflow_);
}

static inline void labstor_unordered_map_{S_NAME}_{T_NAME}_Init(
        struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, void *base_region,
        void *region, uint32_t region_size, uint32_t max_collisions) {
    uint32_t overflow_region_size;
    uint32_t bucket_region_size;
    int i;

    map->base_region_ = base_region;
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

static inline void labstor_unordered_map_{S_NAME}_{T_NAME}_Attach(
        struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, void *base_region, void *region) {
    map->base_region_ = base_region;
    labstor_array_{BUCKET_T_NAME}_Attach(&map->buckets_, region);
    region = labstor_array_{BUCKET_T_NAME}_GetNextSection(&map->buckets_);
    labstor_array_{BUCKET_T_NAME}_Attach(&map->overflow_, region);
}

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_Set(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, struct {BUCKET_T_NAME} *bucket) {
    uint32_t b;
    int i;

    b = {KeyHash}({GetKey}(bucket, map->base_region_), map->base_region_) % labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_);
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

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_Find(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, {S} key, {T} *value) {
    uint32_t b = {KeyHash}(key, map->base_region_) % labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicGetValueByKey(map->base_region_, &map->buckets_, b, key, value)) { return true; }

    //Check the collisions second
    for(i = 0; i < labstor_array_{BUCKET_T_NAME}_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicGetValueByKey(map->base_region_, &map->overflow_, i, key, value)) { return true; }
    }

    return false;
}

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_Remove(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, {S} key) {
    int32_t b = {KeyHash}(key, map->base_region_) % labstor_array_{BUCKET_T_NAME}_GetLength(&map->buckets_);
    int i;

    //Check the primary map first
    if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicNullifyKey(map->base_region_, &map->buckets_, b, key)) { return true;}

    //Check the collision set second
    for(i = 0; i < labstor_array_{BUCKET_T_NAME}_GetLength(&map->overflow_); ++i) {
        if(labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicNullifyKey(map->base_region_, &map->overflow_, i, key)) { return true;}
    }

    return false;
}

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicSetKeyValue(struct labstor_array_{BUCKET_T_NAME} *arr, int i, struct {BUCKET_T_NAME} *bucket) {
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

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicGetValueByKey(void *base_region, struct labstor_array_{BUCKET_T_NAME} *arr, int i, {S} key, {T} *value) {
    struct {BUCKET_T_NAME} tmp;
    do {
        tmp = labstor_array_{BUCKET_T_NAME}_Get(arr, i);
        if (!{KeyCompare}(
                {GetKey}(&tmp, base_region),
                key)
        ) { return false; }
        *value = {GetValue}(&tmp, base_region);
    } while(!__atomic_compare_exchange_n(
            {GetAtomicKeyRef}(labstor_array_{BUCKET_T_NAME}_GetPtr(arr, i)),
            {GetAtomicKeyRef}(&tmp),
            {GetAtomicKey}(&tmp),
            false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_AtomicNullifyKey(void *base_region, struct labstor_array_{BUCKET_T_NAME} *arr, int i, {S} key) {
    struct {BUCKET_T_NAME} tmp;
    do {
        tmp = labstor_array_{BUCKET_T_NAME}_Get(arr, i);
        if (!{KeyCompare}(
                {GetKey}(&tmp, base_region),
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
    typedef labstor_unordered_map_{S_NAME}_{T_NAME} unordered_map_{S_NAME}_{T_NAME};
}
uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}::GetSize(uint32_t num_buckets, uint32_t max_collisions) {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize_global(num_buckets, max_collisions);
}
uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}::GetSize() {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize(this);
}
void* labstor_unordered_map_{S_NAME}_{T_NAME}::GetRegion() {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetRegion(this);
}
void* labstor_unordered_map_{S_NAME}_{T_NAME}::GetBaseRegion() {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetBaseRegion(this);
}
uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}::GetNumBuckets() {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetNumBuckets(this);
}
uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}::GetOverflow() {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetOverflow(this);
}
void labstor_unordered_map_{S_NAME}_{T_NAME}::Init(void *base_region, void *region, uint32_t region_size, uint32_t max_collisions) {
    labstor_unordered_map_{S_NAME}_{T_NAME}_Init(this, base_region, region, region_size, max_collisions);
}
void labstor_unordered_map_{S_NAME}_{T_NAME}::Attach(void *base_region, void *region) {
    labstor_unordered_map_{S_NAME}_{T_NAME}_Attach(this, base_region, region);
}
int labstor_unordered_map_{S_NAME}_{T_NAME}::Set(struct {BUCKET_T_NAME} &bucket) {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_Set(this, &bucket);
}
int labstor_unordered_map_{S_NAME}_{T_NAME}::Find({S} key, {T} &value) {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_Find(this, key, &value);
}
int labstor_unordered_map_{S_NAME}_{T_NAME}::Remove({S} key) {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_Remove(this, key);
}

#endif

#endif //LABSTOR_UNORDERED_MAP_{S_NAME}_{T_NAME}