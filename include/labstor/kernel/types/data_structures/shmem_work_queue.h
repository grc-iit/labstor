//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_WORK_QUEUE_H
#define LABSTOR_WORK_QUEUE_H

#include <labstor/kernel/types/data_structures/shmem_queue_pair.h>
#include <labstor/kernel/types/data_structures/shmem_ring_buffer_qp_ptr.h>

struct labstor_work_queue {
    struct labstor_ring_buffer_qp_ptr queue_;
};

static inline void* labstor_work_queue_GetRegion(struct labstor_work_queue *work_queue) {
    return labstor_ring_buffer_qp_ptr_GetRegion(&work_queue->queue_);
}

static inline uint32_t labstor_work_queue_GetSize_global(uint32_t num_buckets) {
    return labstor_ring_buffer_qp_ptr_GetSize_global(num_buckets);
}
static inline uint32_t labstor_work_queue_GetSize(struct labstor_work_queue *work_queue) {
    return labstor_ring_buffer_qp_ptr_GetSize(&work_queue->queue_);
}

static inline void labstor_work_queue_Init(struct labstor_work_queue *work_queue, void *region, uint32_t region_size) {
    labstor_ring_buffer_qp_ptr_Init(&work_queue->queue_, region, region_size, 0);
}

static inline void labstor_work_queue_Attach(struct labstor_work_queue *work_queue, void *region) {
    labstor_ring_buffer_qp_ptr_Attach(&work_queue->queue_, region);
}

static inline bool labstor_work_queue_Enqueue(struct labstor_work_queue *work_queue, struct labstor_queue_pair *qp, void *base) {
    struct labstor_queue_pair_ptr ptr;
    uint32_t req_id;
    labstor_queue_pair_GetPointer(qp, &ptr, base);
    return labstor_ring_buffer_qp_ptr_Enqueue(&work_queue->queue_, ptr, &req_id);
}

static inline bool labstor_work_queue_Dequeue(struct labstor_work_queue *work_queue, struct labstor_queue_pair *qp, void *base) {
    struct labstor_queue_pair_ptr ptr;
    if(!labstor_ring_buffer_qp_ptr_Dequeue(&work_queue->queue_, &ptr)) {
        return false;
    }
    labstor_queue_pair_Attach(qp, &ptr, base);
    return true;
}

static inline uint32_t labstor_work_queue_GetDepth(struct labstor_work_queue *work_queue) {
    return labstor_ring_buffer_qp_ptr_GetDepth(&work_queue->queue_);
}

#endif //LABSTOR_WORK_QUEUE_H
