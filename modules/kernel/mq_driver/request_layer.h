//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_REQUEST_LAYER_H
#define LABSTOR_REQUEST_LAYER_H

#define REQEUST_LAYER_PKG_ID "REQUEST_LAYER_PKG"

#ifdef __cpluplus
namespace labstor::drivers {
enum class MQOps {
    kSubmit
}
};
#endif

#ifdef KERNEL_BUILD
enum {
    MQ_SUBMIT_REQUEST
};
#endif

struct request_layer_request {
    int op_;
    int dev_id_;
    char *user_buf_;
    size_t lba_;
    size_t off_;
    int hctx_;
};



#endif //LABSTOR_REQUEST_LAYER_H
