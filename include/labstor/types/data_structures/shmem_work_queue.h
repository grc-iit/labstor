//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_WORK_QUEUE_H
#define LABSTOR_WORK_QUEUE_H

#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_qp_ptr.h>

struct labstor_work_queue {
    struct labstor_ring_buffer_qp_ptr queue_;
};

#ifdef __cplusplus
namespace labstor::ipc {

struct work_queue : public ring_buffer_qp_ptr {
};

}
#endif

#endif //LABSTOR_WORK_QUEUE_H
