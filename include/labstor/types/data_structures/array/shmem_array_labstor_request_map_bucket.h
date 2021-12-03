
//labstor_request_map_bucket: The semantic name of the type
//struct labstor_request_map_bucket: The type of the array

#ifndef LABSTOR_ARRAY_labstor_request_map_bucket_H
#define LABSTOR_ARRAY_labstor_request_map_bucket_H

#include <labstor/types/basics.h>

struct labstor_array_labstor_request_map_bucket_header {
    uint32_t length_;
};

struct labstor_array_labstor_request_map_bucket {
    struct labstor_array_labstor_request_map_bucket_header *header_;
    struct labstor_request_map_bucket *arr_;
};

static inline uint32_t labstor_array_labstor_request_map_bucket_GetSize_global(uint32_t length) {
    return sizeof(struct labstor_array_labstor_request_map_bucket_header*) + sizeof(struct labstor_request_map_bucket)*length;
}

static inline uint32_t labstor_array_labstor_request_map_bucket_GetSize(struct labstor_array_labstor_request_map_bucket *arr) {
    return labstor_array_labstor_request_map_bucket_GetSize_global(arr->header_->length_);
}

static inline uint32_t labstor_array_labstor_request_map_bucket_GetLength(struct labstor_array_labstor_request_map_bucket *arr) {
    return arr->header_->length_;
}

static inline void* labstor_array_labstor_request_map_bucket_GetRegion(struct labstor_array_labstor_request_map_bucket *arr) {
    return arr->header_;
}

static inline void* labstor_array_labstor_request_map_bucket_GetNextSection(struct labstor_array_labstor_request_map_bucket *arr) {
    return (char*)arr->header_ + labstor_array_labstor_request_map_bucket_GetSize(arr);
}

static inline void labstor_array_labstor_request_map_bucket_Init(struct labstor_array_labstor_request_map_bucket *arr, void *region, uint32_t region_size, uint32_t length) {
    arr->header_ = (struct labstor_array_labstor_request_map_bucket_header*)region;
    if(length) {
        arr->header_->length_ = length;
    } else {
        arr->header_->length_ = (region_size - sizeof(struct labstor_array_labstor_request_map_bucket_header*)) / sizeof(struct labstor_request_map_bucket);
    }
    arr->arr_ = (struct labstor_request_map_bucket*)(arr->header_ + 1);
}

static inline void labstor_array_labstor_request_map_bucket_Attach(struct labstor_array_labstor_request_map_bucket *arr, void *region) {
    arr->header_ = (struct labstor_array_labstor_request_map_bucket_header*)region;
    arr->arr_ = (struct labstor_request_map_bucket*)(arr->header_ + 1);
}

static inline struct labstor_request_map_bucket labstor_array_labstor_request_map_bucket_Get(struct labstor_array_labstor_request_map_bucket *arr, int i) {
    return arr->arr_[i];
}

static inline struct labstor_request_map_bucket* labstor_array_labstor_request_map_bucket_GetPtr(struct labstor_array_labstor_request_map_bucket *arr, int i) {
    return arr->arr_ + i;
}


#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
namespace labstor::ipc {

struct array_labstor_request_map_bucket_header {
    uint32_t length_;
};

class array_labstor_request_map_bucket : private labstor_array_labstor_request_map_bucket, public shmem_type {
public:
    inline static uint32_t GetSize(uint32_t length) {
        return labstor_array_labstor_request_map_bucket_GetSize_global(length);
    }
    inline uint32_t GetSize() {
        return labstor_array_labstor_request_map_bucket_GetSize(this);
    }
    inline uint32_t GetLength() {
        return labstor_array_labstor_request_map_bucket_GetLength(this);
    }
    inline void* GetRegion() { return labstor_array_labstor_request_map_bucket_GetRegion(this); }

    inline void Init(void *region, uint32_t region_size, uint32_t length = 0) {
        labstor_array_labstor_request_map_bucket_Init(this, region, region_size, length);
    }

    inline void Attach(void *region) {
        labstor_array_labstor_request_map_bucket_Attach(this, region);
    }

    inline struct labstor_request_map_bucket& operator [] (int i) { return arr_[i]; }
};

}

#endif

#endif