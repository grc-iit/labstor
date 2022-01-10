//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_SHMEM_TYPE_H
#define LABSTOR_SHMEM_TYPE_H

#ifdef __cplusplus

#include <stdint.h>

namespace labstor {

class shmem_type {
public:
    inline void* GetNextSection() { return (void*)((char*)GetRegion() + GetSize()); }

    inline virtual void *GetRegion() = 0;
    inline virtual uint32_t GetSize() = 0;
};

}

#endif

#endif //LABSTOR_SHMEM_TYPE_H
