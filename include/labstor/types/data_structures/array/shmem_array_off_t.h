
//off_t
//labstor_off_t: The type of the array

#ifndef LABSTOR_ARRAY_off_t_H
#define LABSTOR_ARRAY_off_t_H

#include <labstor/types/basics.h>

struct labstor_array_off_t_header {
    uint32_t length_;
};

struct labstor_array_off_t {
    struct labstor_array_off_t_header *header_;
    labstor_off_t *arr_;
};

static inline uint32_t labstor_array_off_t_GetSize_global(uint32_t length) {
    return sizeof(struct labstor_array_off_t_header*) + sizeof(labstor_off_t)*length;
}

static inline uint32_t labstor_array_off_t_GetSize(struct labstor_array_off_t *arr) {
    return labstor_array_off_t_GetSize_global(arr->header_->length_);
}

static inline uint32_t labstor_array_off_t_GetLength(struct labstor_array_off_t *arr) {
    return arr->header_->length_;
}

static inline void* labstor_array_off_t_GetRegion(struct labstor_array_off_t *arr) {
    return arr->header_;
}

static inline void* labstor_array_off_t_GetNextSection(struct labstor_array_off_t *arr) {
    return (char*)arr->header_ + labstor_array_off_t_GetSize(arr);
}

static inline void labstor_array_off_t_Init(struct labstor_array_off_t *arr, void *region, uint32_t region_size, uint32_t length) {
    arr->header_ = (struct labstor_array_off_t_header*)region;
    if(length) {
        arr->header_->length_ = length;
    } else {
        arr->header_->length_ = (region_size - sizeof(struct labstor_array_off_t_header*)) / sizeof(labstor_off_t);
    }
    arr->arr_ = (labstor_off_t*)(arr->header_ + 1);
}

static inline void labstor_array_off_t_Attach(struct labstor_array_off_t *arr, void *region) {
    arr->header_ = (struct labstor_array_off_t_header*)region;
    arr->arr_ = (labstor_off_t*)(arr->header_ + 1);
}

static inline labstor_off_t labstor_array_off_t_Get(struct labstor_array_off_t *arr, int i) {
    return arr->arr_[i];
}

static inline labstor_off_t* labstor_array_off_t_GetPtr(struct labstor_array_off_t *arr, int i) {
    return arr->arr_ + i;
}

#endif