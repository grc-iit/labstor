
//labstor_qid_t: The key's real type
//labstor_qid_t: The semantic name of type S
//labstor_qid_t: How the key is stored in the bucket
//labstor::ipc::queue_pair*: The value's type
//qp: The semantic name of type T
//labstor_labstor_qid_t_qp_bucket: The semantic name of the bucket type
//labstor_labstor_qid_t_qp_bucket_hash(labstor::ipc::queue_pair* key, void *region)
//labstor_labstor_qid_t_qp_bucket_GetKey
//labstor_labstor_qid_t_qp_bucket_KeyCompare

#ifndef LABSTOR_UNORDERED_MAP_labstor_qid_t_qp
#define LABSTOR_UNORDERED_MAP_labstor_qid_t_qp

#include <labstor/constants/macros.h>
#include <labstor/types/data_structures/mpmc/bit2map.h>

#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
#endif

struct labstor_unordered_map_labstor_qid_t_qp_header {
    uint32_t num_buckets_;
    uint32_t max_collisions_;
    labstor_bit2map_t bitmap_[];
};

#ifdef __cplusplus
struct labstor_unordered_map_labstor_qid_t_qp : public labstor::shmem_type {
#else
struct labstor_unordered_map_labstor_qid_t_qp {
#endif
    struct labstor_unordered_map_labstor_qid_t_qp_header *header_;
    void *base_region_;
    uint32_t num_buckets_;
    struct labstor_labstor_qid_t_qp_bucket *buckets_;
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

static inline uint32_t labstor_unordered_map_labstor_qid_t_qp_GetHeaderSize(uint32_t num_buckets) {
    return sizeof(struct labstor_unordered_map_labstor_qid_t_qp_header) + labstor_bit2map_GetSize(num_buckets);
}

static inline uint32_t labstor_unordered_map_labstor_qid_t_qp_GetSize_global(uint32_t num_buckets) {
    return labstor_unordered_map_labstor_qid_t_qp_GetHeaderSize(num_buckets) + sizeof(struct labstor_labstor_qid_t_qp_bucket)*num_buckets;
}

static inline uint32_t labstor_unordered_map_labstor_qid_t_qp_GetSize(struct labstor_unordered_map_labstor_qid_t_qp *map) {
    return labstor_unordered_map_labstor_qid_t_qp_GetSize_global(map->num_buckets_);
}

static inline void* labstor_unordered_map_labstor_qid_t_qp_GetRegion(struct labstor_unordered_map_labstor_qid_t_qp *map) {
    return (void*)map->header_;
}

static inline void* labstor_unordered_map_labstor_qid_t_qp_GetBaseRegion(struct labstor_unordered_map_labstor_qid_t_qp *map) {
    return map->base_region_;
}

static inline uint32_t labstor_unordered_map_labstor_qid_t_qp_GetNumBuckets(struct labstor_unordered_map_labstor_qid_t_qp *map) {
    return map->num_buckets_;
}

static inline bool labstor_unordered_map_labstor_qid_t_qp_Init(
        struct labstor_unordered_map_labstor_qid_t_qp *map,
        void *base_region, void *region, uint32_t region_size, uint32_t num_buckets, uint32_t max_collisions) {
    map->base_region_ = base_region;
    map->header_ = (struct labstor_unordered_map_labstor_qid_t_qp_header*)region;
    if(region_size < labstor_unordered_map_labstor_qid_t_qp_GetSize_global(num_buckets)) {
#ifdef __cplusplus
        throw labstor::INVALID_UNORDERED_MAP_SIZE.format(region_size, num_buckets);
#else
        return false;
#endif
    }
    if(num_buckets == 0) {
        num_buckets = region_size - sizeof(struct labstor_unordered_map_labstor_qid_t_qp_header);
        num_buckets *= LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK;
        num_buckets /= (sizeof(struct labstor_labstor_qid_t_qp_bucket)*LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK + sizeof(labstor_bit2map_t));
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
    map->buckets_ = (struct labstor_labstor_qid_t_qp_bucket*)(labstor_bit2map_GetNextSection(map->bitmap_, map->num_buckets_));
    return true;
}

static inline void labstor_unordered_map_labstor_qid_t_qp_Attach(
        struct labstor_unordered_map_labstor_qid_t_qp *map, void *base_region, void *region) {
    map->base_region_ = base_region;
    map->header_ = (struct labstor_unordered_map_labstor_qid_t_qp_header*)region;
    map->buckets_ = (struct labstor_labstor_qid_t_qp_bucket*)(map->header_ + 1);
    map->num_buckets_ = map->header_->num_buckets_;
    map->bitmap_ = map->header_->bitmap_;
    map->buckets_ = (struct labstor_labstor_qid_t_qp_bucket*)(labstor_bit2map_GetNextSection(map->bitmap_, map->num_buckets_));
}

static inline int labstor_unordered_map_labstor_qid_t_qp_Set(struct labstor_unordered_map_labstor_qid_t_qp *map, struct labstor_labstor_qid_t_qp_bucket *bucket) {
    uint32_t b;
    int i, iters = map->header_->max_collisions_ + 1;
    b = labstor_labstor_qid_t_qp_bucket_hash(labstor_labstor_qid_t_qp_bucket_GetKey(bucket, map->base_region_), map->base_region_) % map->num_buckets_;
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

static inline int labstor_unordered_map_labstor_qid_t_qp_Find(struct labstor_unordered_map_labstor_qid_t_qp *map, labstor_qid_t key, labstor::ipc::queue_pair* *value) {
    int i, iters = map->header_->max_collisions_ + 1;
    uint32_t b = labstor_labstor_qid_t_qp_bucket_hash(key, map->base_region_) % map->num_buckets_;
    for(i = 0; i < iters; ++i) {
        if(labstor_bit2map_IsSet(map->bitmap_, b, LABSTOR_BIT2MAP_VALID)) {
            if(labstor_labstor_qid_t_qp_bucket_KeyCompare(labstor_labstor_qid_t_qp_bucket_GetKey(&map->buckets_[b], map->base_region_), key)) {
                *value = labstor_labstor_qid_t_qp_bucket_GetValue(&map->buckets_[b], map->base_region_);
                return true;
            }
        }
        b = (b+1)%map->num_buckets_;
    }
    return false;
}

static inline int labstor_unordered_map_labstor_qid_t_qp_Remove(struct labstor_unordered_map_labstor_qid_t_qp *map, labstor_qid_t key) {
    uint32_t b = labstor_labstor_qid_t_qp_bucket_hash(key, map->base_region_) % map->num_buckets_;
    int i, iters = map->header_->max_collisions_ + 1;
    int num_failed = 0;

    while(num_failed != iters) {
        for(i = 0; i < iters; ++i) {
            if(labstor_bit2map_BeginRemove(map->bitmap_, b)) {
                if(labstor_labstor_qid_t_qp_bucket_KeyCompare(labstor_labstor_qid_t_qp_bucket_GetKey(&map->buckets_[b], map->base_region_), key)) {
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
    typedef labstor_unordered_map_labstor_qid_t_qp unordered_map_labstor_qid_t_qp;
}
uint32_t labstor_unordered_map_labstor_qid_t_qp::GetSize(uint32_t num_buckets) {
    return labstor_unordered_map_labstor_qid_t_qp_GetSize_global(num_buckets);
}
uint32_t labstor_unordered_map_labstor_qid_t_qp::GetSize() {
    return labstor_unordered_map_labstor_qid_t_qp_GetSize(this);
}
void* labstor_unordered_map_labstor_qid_t_qp::GetRegion() {
    return labstor_unordered_map_labstor_qid_t_qp_GetRegion(this);
}
void* labstor_unordered_map_labstor_qid_t_qp::GetBaseRegion() {
    return labstor_unordered_map_labstor_qid_t_qp_GetBaseRegion(this);
}
uint32_t labstor_unordered_map_labstor_qid_t_qp::GetNumBuckets() {
    return labstor_unordered_map_labstor_qid_t_qp_GetNumBuckets(this);
}
void labstor_unordered_map_labstor_qid_t_qp::Init(void *base_region, void *region, uint32_t region_size, uint32_t num_buckets, uint32_t max_collisions) {
    labstor_unordered_map_labstor_qid_t_qp_Init(this, base_region, region, region_size, num_buckets, max_collisions);
}
void labstor_unordered_map_labstor_qid_t_qp::Init(void *base_region, void *region, uint32_t region_size, uint32_t max_collisions) {
    labstor_unordered_map_labstor_qid_t_qp_Init(this, base_region, region, region_size, 0, max_collisions);
}
void labstor_unordered_map_labstor_qid_t_qp::Attach(void *base_region, void *region) {
    labstor_unordered_map_labstor_qid_t_qp_Attach(this, base_region, region);
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