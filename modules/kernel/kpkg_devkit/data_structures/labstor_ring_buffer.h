//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_struct labstor_ring_buffer_H
#define LABSTOR_struct labstor_ring_buffer_H

#include <labstor/types/basics.h>

struct labstor_ring_buffer_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
};

struct labstor_ring_buffer {
    struct labstor_ring_buffer_header *rbuf->header_;
    char *queue_;
};

#endif //LABSTOR_struct labstor_ring_buffer_H
