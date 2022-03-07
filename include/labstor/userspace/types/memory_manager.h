//
// Created by lukemartinlogan on 3/5/22.
//

#ifndef LABSTOR_MEMORY_MANAGER_H
#define LABSTOR_MEMORY_MANAGER_H

#include <labstor/types/allocator/allocator.h>
#include <labstor/types/allocator/segment_allocator.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>

namespace labstor {

class MemoryManager {
private:
    labstor::GenericAllocator *private_alloc_;
    labstor::GenericAllocator *shmem_alloc_;
    labstor::segment_allocator *qp_alloc_;
public:
    void SetPrivateAlloc(labstor::GenericAllocator *private_alloc) {
        private_alloc_ = private_alloc;
    }
    void SetShmemAlloc(labstor::GenericAllocator *shmem_alloc) {
        shmem_alloc_ = shmem_alloc;
    }
    void SetQueueAlloc(labstor::segment_allocator *qp_alloc) {
        qp_alloc_ = qp_alloc;
    }
    void *GetRegion(labstor_qid_flags_t flags) {
        if(LABSTOR_QP_IS_SHMEM(flags)) {
            return shmem_alloc_->GetRegion();
        } else {
            return private_alloc_->GetRegion();
        }
    }

    template<typename T=void*>
    T* AllocShmemQueue(size_t size) {
        return reinterpret_cast<T*>(qp_alloc_->Alloc(size));
    }
    template<typename T=void*>
    T* AllocPrivateQueue(size_t size) {
        return reinterpret_cast<T*>(private_alloc_->Alloc(size));
    }

    template<typename T>
    inline T* AllocRequest(labstor_qid_flags_t flags, uint32_t size) {
        if(LABSTOR_QP_IS_SHMEM(flags)) {
            return reinterpret_cast<T*>(shmem_alloc_->Alloc(size, labstor::ThreadLocal::GetTid()));
        } else {
            return reinterpret_cast<T*>(private_alloc_->Alloc(size, labstor::ThreadLocal::GetTid()));
        }
    }
    template<typename T>
    inline T* AllocRequest(labstor::ipc::qid_t qid, uint32_t size) {
        return AllocRequest<T>(qid.flags_, size);
    }
    template<typename T>
    inline T* AllocRequest(labstor::queue_pair *qp, uint32_t size) {
        return AllocRequest<T>(qp->GetQID(), size);
    }
    template<typename T>
    inline T* AllocRequest(labstor::queue_pair *qp) {
        return AllocRequest<T>(qp->GetQID(), sizeof(T));
    }

    template<typename T>
    inline void FreeRequest(labstor_qid_flags_t flags, T *rq) {
        if(LABSTOR_QP_IS_SHMEM(flags)) {
            shmem_alloc_->Free(rq);
        } else {
            private_alloc_->Free(rq);
        }
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qid_t qid, T *rq) {
        FreeRequest<T>(qid.flags_, rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qtok_t &qtok, T *rq) {
        FreeRequest(qtok.qid_, rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::queue_pair *qp, T *rq) {
        FreeRequest<T>(qp->GetQID(), rq);
    }
};

}

#endif //LABSTOR_MEMORY_MANAGER_H
