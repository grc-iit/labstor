//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_KERNEL_SERVER_TYPES_H
#define LABSTOR_KERNEL_SERVER_TYPES_H

struct package {
    char pkg_id_[256];
    uint32_t runtime_id_;
    void (*process_request_fn)(struct queue_pair *qp, void *request);
    void* (*get_ops)(void);
    int req_size;
};

struct labstor_id {
    char key[256];
};

struct km_request {
    struct labstor_id pkg_id_;
    int code;
};

#endif //LABSTOR_PACKAGE_H
