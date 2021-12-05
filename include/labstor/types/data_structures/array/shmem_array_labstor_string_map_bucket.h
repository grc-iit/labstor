
//labstor_string_map_bucket: The semantic name of the type
//struct labstor_string_map_bucket: The type of the array

#ifndef LABSTOR_ARRAY_labstor_string_map_bucket_H
#define LABSTOR_ARRAY_labstor_string_map_bucket_H

#include <labstor/types/basics.h>
#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#endif

struct labstor_array_labstor_string_map_bucket_header {
    uint32_t length_;
};

#ifdef __cplusplus
struct labstor_array_labstor_string_map_bucket : public labstor::shmem_type {
#else
struct labstor_array_labstor_string_map_bucket {
#endif
    struct labstor_array_labstor_string_map_bucket_header *header_;
    struct labstor_string_map_bucket *arr_;

#ifdef __cplusplus
    inline static uint32_t GetSize(uint32_t length);
    inline uint32_t GetSize();
    inline uint32_t GetLength();
    inline void* GetRegion();
    inline void Init(void *region, uint32_t region_size, uint32_t length = 0);
    inline void Attach(void *region);
    inline struct labstor_string_map_bucket& operator [] (int i) { return arr_[i]; }
#endif
};

static inline uint32_t labstor_array_labstor_string_map_bucket_GetSize_global(uint32_t length) {
    return sizeof(struct labstor_array_labstor_string_map_bucket_header) + sizeof(struct labstor_string_map_bucket)*length;
}

static inline uint32_t labstor_array_labstor_string_map_bucket_GetSize(struct labstor_array_labstor_string_map_bucket *arr) {
    return labstor_array_labstor_string_map_bucket_GetSize_global(arr->header_->length_);
}

static inline uint32_t labstor_array_labstor_string_map_bucket_GetLength(struct labstor_array_labstor_string_map_bucket *arr) {
    return arr->header_->length_;
}

static inline void* labstor_array_labstor_string_map_bucket_GetRegion(struct labstor_array_labstor_string_map_bucket *arr) {
    return arr->header_;
}

static inline void* labstor_array_labstor_string_map_bucket_GetNextSection(struct labstor_array_labstor_string_map_bucket *arr) {
    return (char*)arr->header_ + labstor_array_labstor_string_map_bucket_GetSize(arr);
}

static inline void labstor_array_labstor_string_map_bucket_Init(struct labstor_array_labstor_string_map_bucket *arr, void *region, uint32_t region_size, uint32_t length) {
    arr->header_ = (struct labstor_array_labstor_string_map_bucket_header*)region;
    if(length) {
        arr->header_->length_ = length;
    } else {
        arr->header_->length_ = (region_size - sizeof(struct labstor_array_labstor_string_map_bucket_header*)) / sizeof(struct labstor_string_map_bucket);
    }
    arr->arr_ = (struct labstor_string_map_bucket*)(arr->header_ + 1);
}

static inline void labstor_array_labstor_string_map_bucket_Attach(struct labstor_array_labstor_string_map_bucket *arr, void *region) {
    arr->header_ = (struct labstor_array_labstor_string_map_bucket_header*)region;
    arr->arr_ = (struct labstor_string_map_bucket*)(arr->header_ + 1);
}

static inline struct labstor_string_map_bucket labstor_array_labstor_string_map_bucket_Get(struct labstor_array_labstor_string_map_bucket *arr, int i) {
    return arr->arr_[i];
}

static inline struct labstor_string_map_bucket* labstor_array_labstor_string_map_bucket_GetPtr(struct labstor_array_labstor_string_map_bucket *arr, int i) {
    return arr->arr_ + i;
}


#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_array_labstor_string_map_bucket array_labstor_string_map_bucket;
}
uint32_t labstor_array_labstor_string_map_bucket::GetSize(uint32_t length) {
    return labstor_array_labstor_string_map_bucket_GetSize_global(length);
}
uint32_t labstor_array_labstor_string_map_bucket::GetSize() {
    return labstor_array_labstor_string_map_bucket_GetSize(this);
}
uint32_t labstor_array_labstor_string_map_bucket::GetLength() {
    return labstor_array_labstor_string_map_bucket_GetLength(this);
}
void* labstor_array_labstor_string_map_bucket::GetRegion() {
    return labstor_array_labstor_string_map_bucket_GetRegion(this);
}
void labstor_array_labstor_string_map_bucket::Init(void *region, uint32_t region_size, uint32_t length) {
    labstor_array_labstor_string_map_bucket_Init(this, region, region_size, length);
}
void labstor_array_labstor_string_map_bucket::Attach(void *region) {
    labstor_array_labstor_string_map_bucket_Attach(this, region);
}

#endif

#endif