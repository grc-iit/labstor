//
// Created by lukemartinlogan on 11/12/21.
//

#ifndef LABSTOR_SHMEM_REQUEST_QUEUE_H
#define LABSTOR_SHMEM_REQUEST_QUEUE_H

#include <labstor/types/basics.h>

#ifdef KERNEL_BUILD
#include <linux/types.h>
#endif

//#ifdef __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <labstor/types/data_structures/ring_buffer.h>
#include <labstor/types/data_structures/shmem_request_queue.h>

namespace labstor {

struct work_queue_header {
    ring_buffer_header queue_;
};

class work_queue {
private:
    void *region_;
    work_queue_header *header_;
    shmem_ring_buffer queue_;
public:
    void Init(void *region, uint32_t region_size) {
        region_ = region;
        header_ = (work_queue_header*)region_;
        queue_.Init(&header_->queue_, region_size);
    }
    void Attach(void *region) {
        region_ = region;
        header_ = (work_queue_header*)region;
        queue_.Attach(region_);
    }
    bool Enqueue(request_queue *rq) {
        return queue_.Enqueue(rq);
    }
    request_queue* Dequeue() {
        return (request_queue*)queue_.Dequeue();
    }
};

#endif //LABSTOR_SHMEM_REQUEST_QUEUE_H
