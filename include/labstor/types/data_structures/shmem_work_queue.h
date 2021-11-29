//
// Created by lukemartinlogan on 11/12/21.
//

#ifndef LABSTOR_SHMEM_WORK_QUEUE_H
#define LABSTOR_SHMEM_WORK_QUEUE_H

#include <labstor/types/basics.h>

#ifdef KERNEL_BUILD
#include <linux/types.h>
#endif

//#ifdef __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <labstor/constants/macros.h>
#include <labstor/types/data_structures/shmem_ring_buffer.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include "labstor/types/shmem_type.h"

#include <labstor/userspace_server/macros.h>
#include <labstor/userspace_server/ipc_manager.h>

namespace labstor::ipc {

class work_queue : public shmem_type {
private:
    ring_buffer<queue_pair_ptr> queue_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    work_queue() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    inline void* GetRegion() override { return queue_.GetRegion(); }
    static uint32_t GetSize(uint32_t num_buckets) {
        return ring_buffer<queue_pair_ptr>::GetSize(num_buckets);
    }
    inline uint32_t GetSize() override {
        return queue_.GetSize();
    }

    void Init(void *region, uint32_t region_size) {
        queue_.Init(region, region_size);
    }

    void Attach(void *region) {
        queue_.Attach(region);
    }

    bool Enqueue(queue_pair &qp, void *base) {
        queue_pair_ptr ptr;
        qp.GetPointer(ptr, base);
        return queue_.Enqueue(ptr);
    }

    bool Dequeue(queue_pair &qp, void *&base) {
        queue_pair_ptr ptr;
        if(!queue_.Dequeue(ptr)) {
            return false;
        }
        base = ipc_manager_->GetRegion(ptr);
        qp.Attach(ptr, base);
        return true;
    }

    inline uint32_t GetLength() {
        return queue_.GetDepth();
    }
};

}

#endif //LABSTOR_SHMEM_WORK_QUEUE_H
