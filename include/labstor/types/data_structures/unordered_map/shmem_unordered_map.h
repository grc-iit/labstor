
//{S}: The key's real type
//{S_NAME}: The semantic name of type S
//{S_ATOMIC}: How the key is stored in the bucket
//{T}: The value's type
//{T_NAME}: The semantic name of type T
//{BUCKET_T_NAME}: The semantic name of the bucket type
//{KeyHash}({T} key, void *region)
//{GetKey}
//{KeyCompare}

#ifndef LABSTOR_UNORDERED_MAP_{S_NAME}_{T_NAME}
#define LABSTOR_UNORDERED_MAP_{S_NAME}_{T_NAME}

#include <labstor/constants/macros.h>
#include <labstor/types/data_structures/bit2map.h>

#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
#endif

struct labstor_unordered_map_{S_NAME}_{T_NAME}_header {
    uint32_t num_buckets_;
    uint32_t max_collisions_;
    labstor_bit2map_t bitmap_[];
};

#ifdef __cplusplus
struct labstor_unordered_map_{S_NAME}_{T_NAME} : public labstor::shmem_type {
#else
struct labstor_unordered_map_{S_NAME}_{T_NAME} {
#endif
    struct labstor_unordered_map_{S_NAME}_{T_NAME}_header *header_;
    void *base_region_;
    uint32_t num_buckets_;
    {BUCKET_T} *buckets_;
    labstor_bit2map_t *bitmap_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t num_buckets);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void* GetBaseRegion();
    inline uint32_t GetNumBuckets();
    inline void Init(void *base_region, void *region, uint32_t region_size, uint32_t num_buckets, uint32_t max_collisions);
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

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetHeaderSize(uint32_t num_buckets) {
    return sizeof(struct labstor_unordered_map_{S_NAME}_{T_NAME}_header) + labstor_bit2map_GetSize(num_buckets);
}

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize_global(uint32_t num_buckets) {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetHeaderSize(num_buckets) + sizeof(struct {BUCKET_T_NAME})*num_buckets;
}

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize_global(map->num_buckets_);
}

static inline void* labstor_unordered_map_{S_NAME}_{T_NAME}_GetRegion(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return (void*)map->header_;
}

static inline void* labstor_unordered_map_{S_NAME}_{T_NAME}_GetBaseRegion(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return map->base_region_;
}

static inline uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}_GetNumBuckets(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map) {
    return map->num_buckets_;
}

static inline bool labstor_unordered_map_{S_NAME}_{T_NAME}_Init(
        struct labstor_unordered_map_{S_NAME}_{T_NAME} *map,
        void *base_region, void *region, uint32_t region_size, uint32_t num_buckets, uint32_t max_collisions) {
    map->base_region_ = base_region;
    map->header_ = (struct labstor_unordered_map_{S_NAME}_{T_NAME}_header*)region;
    if(region_size < labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize_global(num_buckets)) {
#ifdef __cplusplus
        throw labstor::INVALID_UNORDERED_MAP_SIZE.format(region_size, num_buckets);
#else
        return false;
#endif
    }
    if(num_buckets == 0) {
        num_buckets = region_size - sizeof(struct labstor_unordered_map_{S_NAME}_{T_NAME}_header);
        num_buckets *= LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK;
        num_buckets /= (sizeof(struct {BUCKET_T_NAME})*LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK + sizeof(labstor_bit2map_t));
    }
    if(num_buckets == 0) {
#ifdef __cplusplus
        throw labstor::INVALID_UNORDERED_MAP_SIZE.format(region_size, num_buckets);
#else
        return false;
#endif
    }

    map->header_->num_buckets_ = num_buckets;
    map->header_->max_collisions_ = max_collisions;
    map->num_buckets_ = map->header_->num_buckets_;
    map->bitmap_ = map->header_->bitmap_;
    labstor_bit2map_Init(map->bitmap_, map->num_buckets_);
    map->buckets_ = (struct {BUCKET_T_NAME}*)(labstor_bit2map_GetNextSection(map->bitmap_, map->num_buckets_));
    return true;
}

static inline void labstor_unordered_map_{S_NAME}_{T_NAME}_Attach(
        struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, void *base_region, void *region) {
    map->base_region_ = base_region;
    map->header_ = (struct labstor_unordered_map_{S_NAME}_{T_NAME}_header*)region;
    map->buckets_ = (struct {BUCKET_T_NAME}*)(map->header_ + 1);
    map->num_buckets_ = map->header_->num_buckets_;
    map->bitmap_ = map->header_->bitmap_;
    map->buckets_ = (struct {BUCKET_T_NAME}*)(labstor_bit2map_GetNextSection(map->bitmap_, map->num_buckets_));
}

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_Set(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, struct {BUCKET_T_NAME} *bucket) {
    uint32_t b;
    int i, iters = map->header_->max_collisions_ + 1;
    b = {KeyHash}({GetKey}(bucket, map->base_region_), map->base_region_) % map->num_buckets_;
    for(i = 0; i < iters; ++i) {
        if(labstor_bit2map_BeginModify(map->bitmap_, b)) {
            map->buckets_[b] = *bucket;
            labstor_bit2map_CommitModify(map->bitmap_, b);
            return true;
        }
        b = (b+1)%map->num_buckets_;
    }
    return false;
}

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_Find(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, {S} key, {T} *value) {
    int i, iters = map->header_->max_collisions_ + 1;
    uint32_t b = {KeyHash}(key, map->base_region_) % map->num_buckets_;
    for(i = 0; i < iters; ++i) {
        if(labstor_bit2map_IsSet(map->bitmap_, b, LABSTOR_BIT2MAP_VALID)) {
            if({KeyCompare}({GetKey}(&map->buckets_[b], map->base_region_), key)) {
                *value = {GetValue}(&map->buckets_[b], map->base_region_);
                return true;
            }
        }
        b = (b+1)%map->num_buckets_;
    }
    return false;
}

static inline int labstor_unordered_map_{S_NAME}_{T_NAME}_Remove(struct labstor_unordered_map_{S_NAME}_{T_NAME} *map, {S} key) {
    uint32_t b = {KeyHash}(key, map->base_region_) % map->num_buckets_;
    int i, iters = map->header_->max_collisions_ + 1;
    int num_failed = 0;

    while(num_failed != iters) {
        for(i = 0; i < iters; ++i) {
            if(labstor_bit2map_BeginRemove(map->bitmap_, b)) {
                if({KeyCompare}({GetKey}(&map->buckets_[b], map->base_region_), key)) {
                    labstor_bit2map_CommitRemove(map->bitmap_, b);
                    return true;
                } else {
                    labstor_bit2map_IgnoreRemove(map->bitmap_, b);
                    ++num_failed;
                    break;
                }
            }
            b = (b+1)%map->num_buckets_;
        }
    }
    return false;
}

#ifdef __cplusplus

namespace labstor::ipc {
    typedef labstor_unordered_map_{S_NAME}_{T_NAME} unordered_map_{S_NAME}_{T_NAME};
}
uint32_t labstor_unordered_map_{S_NAME}_{T_NAME}::GetSize(uint32_t num_buckets) {
    return labstor_unordered_map_{S_NAME}_{T_NAME}_GetSize_global(num_buckets);
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
void labstor_unordered_map_{S_NAME}_{T_NAME}::Init(void *base_region, void *region, uint32_t region_size, uint32_t num_buckets, uint32_t max_collisions) {
    labstor_unordered_map_{S_NAME}_{T_NAME}_Init(this, base_region, region, region_size, num_buckets, max_collisions);
}
void labstor_unordered_map_{S_NAME}_{T_NAME}::Init(void *base_region, void *region, uint32_t region_size, uint32_t max_collisions) {
    labstor_unordered_map_{S_NAME}_{T_NAME}_Init(this, base_region, region, region_size, 0, max_collisions);
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