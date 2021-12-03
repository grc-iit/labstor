//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_REQUEST_KERNEL_H
#define LABSTOR_REQUEST_KERNEL_H

#include <labstor/types/basics.h>

struct labstor_netlink_header {
    labstor_runtime_id_t runtime_id_;
};

struct labstor_request {
    uint32_t req_id_;
    uint32_t runtime_id_;
    uint16_t op_;
};

#endif //LABSTOR_REQUEST_KERNEL_H
