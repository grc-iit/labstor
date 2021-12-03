
//uint32_t: The semantic name of the type
//uint32_t: The type of the array

#ifndef LABSTOR_ARRAY_uint32_t_H
#define LABSTOR_ARRAY_uint32_t_H

#include <labstor/types/basics.h>

struct labstor_array_uint32_t_header {
    uint32_t length_;
};

struct labstor_array_uint32_t {
    struct labstor_array_uint32_t_header *header_;
    uint32_t *arr_;
};

static inline uint32_t labstor_array_uint32_t_GetSize_global(uint32_t length) {
    return sizeof(struct labstor_array_uint32_t_header*) + sizeof(uint32_t)*length;
}

static inline uint32_t labstor_array_uint32_t_GetSize(struct labstor_array_uint32_t *arr) {
    return labstor_array_uint32_t_GetSize_global(arr->header_->length_);
}

static inline uint32_t labstor_array_uint32_t_GetLength(struct labstor_array_uint32_t *arr) {
    return arr->header_->length_;
}

static inline void* labstor_array_uint32_t_GetRegion(struct labstor_array_uint32_t *arr) {
    return arr->header_;
}

static inline void* labstor_array_uint32_t_GetNextSection(struct labstor_array_uint32_t *arr) {
    return (char*)arr->header_ + labstor_array_uint32_t_GetSize(arr);
}

static inline void labstor_array_uint32_t_Init(struct labstor_array_uint32_t *arr, void *region, uint32_t region_size, uint32_t length) {
    arr->header_ = (struct labstor_array_uint32_t_header*)region;
    if(length) {
        arr->header_->length_ = length;
    } else {
        arr->header_->length_ = (region_size - sizeof(struct labstor_array_uint32_t_header*)) / sizeof(uint32_t);
    }
    arr->arr_ = (uint32_t*)(arr->header_ + 1);
}

static inline void labstor_array_uint32_t_Attach(struct labstor_array_uint32_t *arr, void *region) {
    arr->header_ = (struct labstor_array_uint32_t_header*)region;
    arr->arr_ = (uint32_t*)(arr->header_ + 1);
}

static inline uint32_t labstor_array_uint32_t_Get(struct labstor_array_uint32_t *arr, int i) {
    return arr->arr_[i];
}

static inline uint32_t* labstor_array_uint32_t_GetPtr(struct labstor_array_uint32_t *arr, int i) {
    return arr->arr_ + i;
}


#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
namespace labstor::ipc {

struct array_uint32_t_header {
    uint32_t length_;
};

class array_uint32_t : private labstor_array_uint32_t, public shmem_type {
public:
    inline static uint32_t GetSize(uint32_t length) {
        return labstor_array_uint32_t_GetSize_global(length);
    }
    inline uint32_t GetSize() {
        return labstor_array_uint32_t_GetSize(this);
    }
    inline uint32_t GetLength() {
        return labstor_array_uint32_t_GetLength(this);
    }
    inline void* GetRegion() { return labstor_array_uint32_t_GetRegion(this); }

    inline void Init(void *region, uint32_t region_size, uint32_t length = 0) {
        labstor_array_uint32_t_Init(this, region, region_size, length);
    }

    inline void Attach(void *region) {
        labstor_array_uint32_t_Attach(this, region);
    }

    inline uint32_t& operator [] (int i) { return arr_[i]; }
};

}

#endif

#endif