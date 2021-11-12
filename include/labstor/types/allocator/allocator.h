//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_GENERIC_ALLOCATOR_H
#define LABSTOR_GENERIC_ALLOCATOR_H

#ifdef __cplusplus

#include <stdint.h>

namespace labstor {

class GenericAllocator {
public:
    virtual void* Alloc(uint32_t size, uint32_t core) = 0;
    virtual void Free(void *data) = 0;
    virtual void Attach(void *region) {};
};

}

#endif

#endif //LABSTOR_GENERIC_ALLOCATOR_H
