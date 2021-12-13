//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_WORK_QUEUE_H
#define LABSTOR_WORK_QUEUE_H

#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_qp_ptr.h>

struct labstor_work_queue {
    struct labstor_ring_buffer_qp_ptr queue_;
};

static inline uint32_t labstor_work_queue_GetSize_global(uint32_t max_depth) {
    return labstor_ring_buffer_qp_ptr_GetSize_global(max_depth);
}

static inline uint32_t labstor_work_queue_GetSize(struct labstor_work_queue *work_queue) {
    return labstor_ring_buffer_qp_ptr_GetSize(&work_queue->queue_);
}

static inline void* labstor_work_queue_GetRegion(struct labstor_work_queue *work_queue) {
    return labstor_ring_buffer_qp_ptr_GetRegion(&work_queue->queue_);
}

static inline void* labstor_work_queue_GetNextSection(struct labstor_work_queue *work_queue) {
    return labstor_ring_buffer_qp_ptr_GetNextSection(&work_queue->queue_);
}

static inline uint32_t labstor_work_queue_GetDepth(struct labstor_work_queue *work_queue) {
    return labstor_ring_buffer_qp_ptr_GetDepth(&work_queue->queue_);
}

static inline uint32_t labstor_work_queue_GetMaxDepth(struct labstor_work_queue *work_queue) {
    return labstor_ring_buffer_qp_ptr_GetMaxDepth(&work_queue->queue_);
}

static inline bool labstor_work_queue_Init(struct labstor_work_queue *work_queue, void *region, uint32_t region_size, uint32_t max_depth) {
    return labstor_ring_buffer_qp_ptr_Init(&work_queue->queue_, region, region_size, max_depth);
}

static inline void labstor_work_queue_Attach(struct labstor_work_queue *work_queue, void *region) {
    labstor_ring_buffer_qp_ptr_Attach(&work_queue->queue_, region);
}

static inline bool labstor_work_queue_Enqueue(struct labstor_work_queue *work_queue, struct labstor_queue_pair_ptr data, uint32_t *req_id) {
    return labstor_ring_buffer_qp_ptr_Enqueue(&work_queue->queue_, data, req_id);
}

static inline bool labstor_work_queue_Enqueue_simple(struct labstor_work_queue *work_queue, struct labstor_queue_pair_ptr data) {
    return labstor_ring_buffer_qp_ptr_Enqueue_simple(&work_queue->queue_, data);
}

static inline bool labstor_work_queue_Dequeue(struct labstor_work_queue *work_queue, struct labstor_queue_pair_ptr *data) {
    return labstor_ring_buffer_qp_ptr_Dequeue(&work_queue->queue_, data);
}

#ifdef __cplusplus
namespace labstor::ipc {

struct work_queue : public ring_buffer_qp_ptr {
};

}
#endif

#endif //LABSTOR_WORK_QUEUE_H
