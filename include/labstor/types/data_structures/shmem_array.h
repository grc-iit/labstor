//
// Created by lukemartinlogan on 11/21/21.
//

#ifndef LABSTOR_SHMEM_ARRAY_H
#define LABSTOR_SHMEM_ARRAY_H

#include "labstor/types/shmem_type.h"

#ifdef __cplusplus

namespace labstor::ipc {

struct array_header {
    uint32_t length_;
};

template<typename T>
class array : public shmem_type {
private:
    array_header *header_;
    T *arr_;
public:
    inline static uint32_t GetSize(uint32_t length) {
        return sizeof(array_header) + sizeof(T)*length;
    }
    inline uint32_t GetSize() {
        return GetSize(header_->length_);
    }
    inline uint32_t GetLength() {
        return header_->length_;
    }
    inline void* GetRegion() { return header_; }

    void Init(void *region, uint32_t region_size, uint32_t length = 0) {
        header_ = (array_header*)region;
        if(length) {
            header_->length_ = length;
        } else {
            header_->length_ = (region_size - sizeof(array_header)) / sizeof(T);
        }
        arr_ = (T*)(header_ + 1);
    }

    void Attach(void *region) {
        header_ = (array_header*)region;
        arr_ = (T*)(header_ + 1);
    }

    T& operator [] (int i) { return arr_[i]; }
};

}

#endif

#endif //LABSTOR_SHMEM_ARRAY_H
