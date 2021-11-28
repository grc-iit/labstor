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

namespace labstor::ipc {

class work_queue : public shmem_type {
private:
    ring_buffer<queue_pair_ptr> queue_;
public:
    static uint32_t GetSize(uint32_t num_buckets) {
        return ring_buffer<queue_pair_ptr>::GetSize(num_buckets);
    }
    inline uint32_t GetSize() {
        return queue_.GetSize();
    }


    void Init(void *region, uint32_t region_size) {
        region_ = region;
        queue_.Init(region_, region_size);
    }

    void Attach(void *region) {
        region_ = region;
        queue_.Attach(region);
    }

    bool Enqueue(queue_pair &qp) {
        queue_pair_ptr ptr;
        ptr.sq_off = LABSTOR_REGION_SUB(qp.sq.GetRegion(), region_);
        ptr.cq_off = LABSTOR_REGION_SUB(qp.cq.GetRegion(), region_);
        return queue_.Enqueue(ptr);
    }

    bool Dequeue(queue_pair &qp) {
        queue_pair_ptr ptr;
        if(!queue_.Dequeue(ptr)) {
            return false;
        }
        qp.sq.Attach(LABSTOR_REGION_ADD(ptr.sq_off, region_));
        qp.cq.Attach(LABSTOR_REGION_ADD(ptr.cq_off, region_));
        return true;
    }

    inline uint32_t GetLength() {
        return queue_.GetDepth();
    }
};

}

#endif //LABSTOR_SHMEM_WORK_QUEUE_H
