
#include <linux/types.h>
#include <labstor/types/basics.h>

//SUFFIX: the name of the file
//${T}: The type of the array

struct labstor_array_header_${SUFFIX} {
    uint32_t length_;
};

struct labstor_array_${SUFFIX} {
    labstor_array_header *header_;
    ${T} *arr_;
};

static inline uint32_t labstor_array_GetSize_global_${SUFFIX}(uint32_t length) {
    return sizeof(struct labstor_array_header*) + sizeof(${T})*length;
}

static inline uint32_t labstor_array_GetSize_${SUFFIX}(labstor_array *arr) {
    return labstor_array_GetSize_global(arr->header_->length_);
}

static inline uint32_t labstor_array_GetLength_${SUFFIX}(labstor_array *arr) {
    return arr->header_->length_;
}

static inline void* labstor_array_GetRegion_${SUFFIX}(labstor_array *arr) {
    return arr->header_;
}

static inline void labstor_array_Init_${SUFFIX}(labstor_array *arr, void *region, uint32_t region_size, uint32_t length = 0) {
    arr->header_ = (struct labstor_array_header*)region;
    if(length) {
        arr->header_->length_ = length;
    } else {
        arr->header_->length_ = (region_size - sizeof(struct labstor_array_header*)) / sizeof(${T});
    }
    arr->arr_ = (${T}*)(arr->header_ + 1);
}

static inline void labstor_array_Attach_${SUFFIX}(labstor_array *arr, void *region) {
    arr->header_ = (struct labstor_array_header*)region;
    arr->arr_ = (${T}*)(arr->header_ + 1);
}

static inline ${T} labstor_array_Get_${SUFFIX}(labstor_array *arr, int i) {
    return arr->arr_[i];
}

static inline ${T}* labstor_array_GetRef_${SUFFIX}(labstor_array *arr, int i) {
    return arr->arr_ + i;
}
