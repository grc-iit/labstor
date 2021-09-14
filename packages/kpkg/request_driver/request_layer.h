//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_REQUEST_LAYER_H
#define LABSTOR_REQUEST_LAYER_H

#define REQEUST_LAYER_PKG_ID "REQUEST_LAYER_PKG"

enum {
    RQ_PKG_SUBMIT_REQUEST,
    RQ_PKG_POLL_REQUEST
};

struct request_layer_request {
    int op;
    char dev[256];
    char *user_buf;
    size_t lba;
    size_t off;
    int hctx;
};

#endif //LABSTOR_REQUEST_LAYER_H
