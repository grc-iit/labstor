
//labstor_qtok_t: The semantic name of the type
//labstor_qtok_t: The type of the array

#ifndef LABSTOR_ARRAY_labstor_qtok_t_H
#define LABSTOR_ARRAY_labstor_qtok_t_H

#include <labstor/types/basics.h>
#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#endif

struct labstor_array_labstor_qtok_t_header {
    uint32_t length_;
};

#ifdef __cplusplus
struct labstor_array_labstor_qtok_t : public labstor::shmem_type {
#else
struct labstor_array_labstor_qtok_t {
#endif
    struct labstor_array_labstor_qtok_t_header *header_;
    labstor_qtok_t *arr_;

#ifdef __cplusplus
    inline static uint32_t GetSize(uint32_t length);
    inline uint32_t GetSize();
    inline uint32_t GetLength();
    inline void* GetRegion();
    inline void Init(void *region, uint32_t region_size, uint32_t length = 0);
    inline void Attach(void *region);
    inline labstor_qtok_t& operator [] (int i) { return arr_[i]; }
#endif
};

static inline uint32_t labstor_array_labstor_qtok_t_GetSize_global(uint32_t length) {
    return sizeof(struct labstor_array_labstor_qtok_t_header) + sizeof(labstor_qtok_t)*length;
}

static inline uint32_t labstor_array_labstor_qtok_t_GetSize(struct labstor_array_labstor_qtok_t *arr) {
    return labstor_array_labstor_qtok_t_GetSize_global(arr->header_->length_);
}

static inline uint32_t labstor_array_labstor_qtok_t_GetLength(struct labstor_array_labstor_qtok_t *arr) {
    return arr->header_->length_;
}

static inline void* labstor_array_labstor_qtok_t_GetRegion(struct labstor_array_labstor_qtok_t *arr) {
    return arr->header_;
}

static inline void* labstor_array_labstor_qtok_t_GetNextSection(struct labstor_array_labstor_qtok_t *arr) {
    return (char*)arr->header_ + labstor_array_labstor_qtok_t_GetSize(arr);
}

static inline void labstor_array_labstor_qtok_t_Init(struct labstor_array_labstor_qtok_t *arr, void *region, uint32_t region_size, uint32_t length) {
    arr->header_ = (struct labstor_array_labstor_qtok_t_header*)region;
    if(length) {
        arr->header_->length_ = length;
    } else {
        arr->header_->length_ = (region_size - sizeof(struct labstor_array_labstor_qtok_t_header*)) / sizeof(labstor_qtok_t);
    }
    arr->arr_ = (labstor_qtok_t*)(arr->header_ + 1);
}

static inline void labstor_array_labstor_qtok_t_Attach(struct labstor_array_labstor_qtok_t *arr, void *region) {
    arr->header_ = (struct labstor_array_labstor_qtok_t_header*)region;
    arr->arr_ = (labstor_qtok_t*)(arr->header_ + 1);
}

static inline labstor_qtok_t labstor_array_labstor_qtok_t_Get(struct labstor_array_labstor_qtok_t *arr, int i) {
    return arr->arr_[i];
}

static inline labstor_qtok_t* labstor_array_labstor_qtok_t_GetPtr(struct labstor_array_labstor_qtok_t *arr, int i) {
    return arr->arr_ + i;
}


#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_array_labstor_qtok_t array_labstor_qtok_t;
}
uint32_t labstor_array_labstor_qtok_t::GetSize(uint32_t length) {
    return labstor_array_labstor_qtok_t_GetSize_global(length);
}
uint32_t labstor_array_labstor_qtok_t::GetSize() {
    return labstor_array_labstor_qtok_t_GetSize(this);
}
uint32_t labstor_array_labstor_qtok_t::GetLength() {
    return labstor_array_labstor_qtok_t_GetLength(this);
}
void* labstor_array_labstor_qtok_t::GetRegion() {
    return labstor_array_labstor_qtok_t_GetRegion(this);
}
void labstor_array_labstor_qtok_t::Init(void *region, uint32_t region_size, uint32_t length) {
    labstor_array_labstor_qtok_t_Init(this, region, region_size, length);
}
void labstor_array_labstor_qtok_t::Attach(void *region) {
    labstor_array_labstor_qtok_t_Attach(this, region);
}

#endif

#endif