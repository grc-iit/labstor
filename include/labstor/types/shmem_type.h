//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_SHMEM_TYPE_H
#define LABSTOR_SHMEM_TYPE_H

namespace labstor {

class shmem_type {
protected:
    void *region_;
public:
    inline void *GetRegion() { return region_; }
    inline void* GetNextSection() { return (void*)((char*)region_ + GetSize()); }
    inline virtual uint32_t GetSize() = 0;
};

}

#endif //LABSTOR_SHMEM_TYPE_H
