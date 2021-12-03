//
// Created by lukemartinlogan on 11/30/21.
//

#ifndef LABSTOR_MODULE_KERNEL_H
#define LABSTOR_MODULE_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_request.h>

typedef void (*process_request_fn_type)(struct labstor_queue_pair *qp, struct labstor_request *rq);
typedef void (*process_request_fn_netlink_type)(int pid, void *rq);

struct labstor_module {
    struct labstor_id module_id;
    labstor_runtime_id_t runtime_id;
    process_request_fn_type process_request_fn;
    process_request_fn_netlink_type process_request_fn_netlink;
};

#endif //LABSTOR_MODULE_KERNEL_H
