//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_MESSAGES_H
#define LABSTOR_MESSAGES_H

#include <labstor/types/data_structures/shmem_request_queue.h>
#include <labstor/types/data_structures/shmem_qtok.h>

namespace labstor::ipc {

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
    uint32_t concurrency;
    uint32_t request_unit;
    uint32_t queue_size;
};

struct register_qp_request : public labstor::ipc::admin_request {
    int count_;
    register_qp_request() {}
    register_qp_request(int count) : count_(count), labstor::ipc::admin_request(LABSTOR_ADMIN_REGISTER_QP) {}
};
typedef admin_reply register_qp_reply;

struct poll_request : public labstor::ipc::request {
    labstor::ipc::qtok_t qtok_;
    labstor::ipc::qtok_set qtoks_;
};

}

#endif //LABSTOR_MESSAGES_H
