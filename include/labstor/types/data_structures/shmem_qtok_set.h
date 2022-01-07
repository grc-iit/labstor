//
// Created by lukemartinlogan on 1/7/22.
//

#ifndef LABSTOR_SHMEM_QTOK_SET_H
#define LABSTOR_SHMEM_QTOK_SET_H

#include <labstor/types/data_structures/mpmc/shmem_ring_buffer.h>
namespace labstor::ipc {
    typedef mpmc::ring_buffer<qtok_t> qtok_set;
}

#endif //LABSTOR_SHMEM_QTOK_SET_H
