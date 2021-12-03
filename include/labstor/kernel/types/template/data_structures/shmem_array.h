
//{SUFFIX}
//{T}: The type of the array

#ifndef LABSTOR_ARRAY_{SUFFIX}_H
#define LABSTOR_ARRAY_{SUFFIX}_H

#include <labstor/types/basics.h>

struct labstor_array_{SUFFIX}_header {
    uint32_t length_;
};

struct labstor_array_{SUFFIX} {
    struct labstor_array_{SUFFIX}_header *header_;
    {T} *arr_;
};

static inline uint32_t labstor_array_{SUFFIX}_GetSize_global(uint32_t length) {
    return sizeof(struct labstor_array_{SUFFIX}_header*) + sizeof({T})*length;
}

static inline uint32_t labstor_array_{SUFFIX}_GetSize(struct labstor_array_{SUFFIX} *arr) {
    return labstor_array_{SUFFIX}_GetSize_global(arr->header_->length_);
}

static inline uint32_t labstor_array_{SUFFIX}_GetLength(struct labstor_array_{SUFFIX} *arr) {
    return arr->header_->length_;
}

static inline void* labstor_array_{SUFFIX}_GetRegion(struct labstor_array_{SUFFIX} *arr) {
    return arr->header_;
}

static inline void* labstor_array_{SUFFIX}_GetNextSection(struct labstor_array_{SUFFIX} *arr) {
    return (char*)arr->header_ + labstor_array_{SUFFIX}_GetSize(arr);
}

static inline void labstor_array_{SUFFIX}_Init(struct labstor_array_{SUFFIX} *arr, void *region, uint32_t region_size, uint32_t length) {
    arr->header_ = (struct labstor_array_{SUFFIX}_header*)region;
    if(length) {
        arr->header_->length_ = length;
    } else {
        arr->header_->length_ = (region_size - sizeof(struct labstor_array_{SUFFIX}_header*)) / sizeof({T});
    }
    arr->arr_ = ({T}*)(arr->header_ + 1);
}

static inline void labstor_array_{SUFFIX}_Attach(struct labstor_array_{SUFFIX} *arr, void *region) {
    arr->header_ = (struct labstor_array_{SUFFIX}_header*)region;
    arr->arr_ = ({T}*)(arr->header_ + 1);
}

static inline {T} labstor_array_{SUFFIX}_Get(struct labstor_array_{SUFFIX} *arr, int i) {
    return arr->arr_[i];
}

static inline {T}* labstor_array_{SUFFIX}_GetRef(struct labstor_array_{SUFFIX} *arr, int i) {
    return arr->arr_ + i;
}

#endif