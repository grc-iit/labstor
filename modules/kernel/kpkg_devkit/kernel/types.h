//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_KERNEL_SERVER_TYPES_H
#define LABSTOR_KERNEL_SERVER_TYPES_H

#include <labstor/types/basics.h>
#include <linux/types.h>
#include "request_queue.h"

typedef void (*process_request_fn_type)(struct labstor_queue_pair *qp, void *request);
typedef void (*process_request_fn_netlink_type)(int pid, void *request);

struct labstor_module {
    struct labstor_id module_id;
    uint32_t runtime_id;
    process_request_fn_type process_request_fn;
    process_request_fn_netlink_type process_request_fn_netlink;
    void* (*get_ops)(void);
    int req_size;
};

struct km_request {
    struct labstor_id module_id;
};

#endif //LABSTOR_MODULE_H
