//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_GENERIC_ALLOCATOR_H
#define LABSTOR_GENERIC_ALLOCATOR_H

#ifdef __cplusplus

#include <labstor/types/shmem_type.h>
#include <stdint.h>
#include <sched.h>
#include <labstor/types/thread_local.h>

namespace labstor {

class GenericAllocator : public shmem_type {
public:
    void* Alloc(uint32_t size) {
        return Alloc(size, labstor::ThreadLocal::GetTid());
    }
    virtual void* Alloc(uint32_t size, uint32_t core) = 0;
    virtual void Free(void *data) = 0;
};

}

#endif

#endif //LABSTOR_GENERIC_ALLOCATOR_H
