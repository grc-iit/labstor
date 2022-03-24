
#ifndef LABSTOR_UNORDERED_MAP_H
#define LABSTOR_UNORDERED_MAP_H

#include "labstor/constants/macros.h"
#include "labstor/types/data_structures/bit2map.h"
#include "labstor/types/shmem_type.h"
#include "labstor/userspace/util/errors.h"

struct unordered_map_header {
    uint32_t num_buckets_;
    uint32_t max_collisions_;
    labstor_bit2map_t bitmap_[];
};

template<typename S, typename T, typename BUCKET_T>
struct unordered_map : public labstor::shmem_type {
    struct unordered_map_header *header_;
    void *base_region_;
    uint32_t num_buckets_;
    BUCKET_T *buckets_;
    labstor_bit2map_t *bitmap_;

    static inline uint32_t GetHeaderSize(uint32_t num_buckets) {
        return sizeof(struct unordered_map_header) + labstor_bit2map_GetSize(num_buckets);
    }

    static inline uint32_t GetSize(uint32_t num_buckets) {
        return GetHeaderSize(num_buckets) + sizeof(BUCKET_T)*num_buckets;
    }

    inline uint32_t GetSize() {
        return GetSize(num_buckets_);
    }

    inline void* GetRegion() {
        return (void*)header_;
    }

    inline void* GetBaseRegion() {
        return base_region_;
    }

    inline uint32_t GetNumBuckets() {
        return num_buckets_;
    }

    inline bool Init(
            void *base_region, void *region, uint32_t region_size, uint32_t num_buckets, uint32_t max_collisions=0) {
        base_region_ = base_region;
        header_ = (struct unordered_map_header*)region;
        if(region_size < GetSize(num_buckets)) {
            throw labstor::INVALID_UNORDERED_MAP_SIZE.format(region_size, num_buckets);
        }
        if(num_buckets == 0) {
            num_buckets = region_size - sizeof(struct unordered_map_header);
            num_buckets *= LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK;
            num_buckets /= (sizeof(BUCKET_T)*LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK + sizeof(labstor_bit2map_t));
        }
        if(num_buckets == 0) {
            throw labstor::INVALID_UNORDERED_MAP_SIZE.format(region_size, num_buckets);
        }

        header_->num_buckets_ = num_buckets;
        header_->max_collisions_ = max_collisions;
        num_buckets_ = header_->num_buckets_;
        bitmap_ = header_->bitmap_;
        labstor_bit2map_Init(bitmap_, num_buckets_);
        buckets_ = (BUCKET_T*)(labstor_bit2map_GetNextSection(bitmap_, num_buckets_));
        return true;
    }

    inline void Attach(
            void *base_region, void *region) {
        base_region_ = base_region;
        header_ = (struct unordered_map_header*)region;
        buckets_ = (BUCKET_T*)(header_ + 1);
        num_buckets_ = header_->num_buckets_;
        bitmap_ = header_->bitmap_;
        buckets_ = (BUCKET_T*)(labstor_bit2map_GetNextSection(bitmap_, num_buckets_));
    }

    inline int Set(BUCKET_T &bucket) {
        uint32_t b;
        int i, iters = header_->max_collisions_ + 1;
        b = BUCKET_T::KeyHash(bucket.GetKey(base_region_), base_region_) % num_buckets_;
        for(i = 0; i < iters; ++i) {
            if(labstor_bit2map_BeginModify(bitmap_, b)) {
                buckets_[b] = bucket;
                labstor_bit2map_CommitModify(bitmap_, b);
                return true;
            }
            b = (b+1)%num_buckets_;
        }
        return false;
    }

    inline int Find(S key, T &value) {
        int i, iters = header_->max_collisions_ + 1;
        uint32_t b = BUCKET_T::KeyHash(key, base_region_) % num_buckets_;
        for(i = 0; i < iters; ++i) {
            if(labstor_bit2map_IsSet(bitmap_, b, LABSTOR_BIT2MAP_VALID)) {
                if(BUCKET_T::KeyCompare(buckets_[b].GetKey(base_region_), key)) {
                    value = buckets_[b].GetValue(base_region_);
                    return true;
                }
            }
            b = (b+1)%num_buckets_;
        }
        return false;
    }

    inline int Remove(S key) {
        uint32_t b = BUCKET_T::KeyHash(key, base_region_) % num_buckets_;
        int i, iters = header_->max_collisions_ + 1;
        int num_failed = 0;
        while(num_failed != iters) {
            for(i = 0; i < iters; ++i) {
                if(labstor_bit2map_BeginRemove(bitmap_, b)) {
                    if(BUCKET_T::KeyCompare(buckets_[b].GetKey(base_region_), key)) {
                        labstor_bit2map_CommitRemove(bitmap_, b);
                        return true;
                    } else {
                        labstor_bit2map_IgnoreRemove(bitmap_, b);
                        ++num_failed;
                        break;
                    }
                }
                b = (b+1)%num_buckets_;
            }
        }
        return false;
    }

    inline T operator [](S key) {
        T value;
        if(Find(key, value)) {
            return value;
        }
        throw labstor::INVALID_UNORDERED_MAP_KEY.format();
    }
};


#endif //LABSTOR_UNORDERED_MAP_H