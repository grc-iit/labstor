//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_MESSAGES_H
#define LABSTOR_MESSAGES_H

#include <labstor/types/data_structures/shmem_request_queue.h>

namespace labstor {

enum {
    LABSTOR_ADMIN_REGISTER_QP
};

struct admin_request {
    int op_;
    admin_request() {}
    admin_request(int op) : op_(op) {}
};

struct admin_reply {
    int code_;
    admin_reply() {}
    admin_reply(int code) : code_(code) {}
};

struct setup_reply : public admin_reply {
    uint32_t region_id;
    uint32_t region_size;
};

struct register_qp_request : public admin_request {
    int count_;
    register_qp_request() {}
    register_qp_request(int count) : count_(count), admin_request(LABSTOR_ADMIN_REGISTER_QP) {}
};
typedef admin_reply register_qp_reply;

}

#endif //LABSTOR_MESSAGES_H
