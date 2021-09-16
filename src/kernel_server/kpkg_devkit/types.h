//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_KERNEL_SERVER_TYPES_H
#define LABSTOR_KERNEL_SERVER_TYPES_H

#include <linux/types.h>
#include "request_queue.h"

struct labstor_id {
    char key[256];
};

struct labstor_module {
    struct labstor_id module_id;
    uint32_t runtime_id;
    void (*process_request_fn)(struct labstor_queue_pair *qp, void *request);
    void* (*get_ops)(void);
    int req_size;
};

struct km_startup_request {
    int code;
    int num_queues;
    size_t queue_size;
    void *starting_address;
};

#endif //LABSTOR_MODULE_H
