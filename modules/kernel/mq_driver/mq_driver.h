//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_REQUEST_LAYER_H
#define LABSTOR_REQUEST_LAYER_H

#define MQ_DRIVER_MODULE_ID "MQ_DRIVER"

#ifdef __cpluplus
namespace labstor::drivers {
enum class MQOps {
    kRegister,
    kWrite,
    kRead,
    kIOComplete
}
};
#endif

#ifdef KERNEL_BUILD
enum {
    LABSTOR_MQ_DRIVER_REGISTER,
    LABSTOR_MQ_DRIVER_WRITE,
    LABSTOR_MQ_DRIVER_READ
};
#endif

struct labstor_submit_mq_driver_request {
    struct labstor_request header_;
    int dev_id_;
    char *user_buf_;
    size_t lba_;
    size_t off_;
    int hctx_;
#ifdef __cplusplus
    inline void Init(int ns_id, Ops op, int dev_id, char *user_buf, size_t lba, size_t off, int hctx) {
        Init(ns_id, static_cast<int>(op);, dev_id, user_buf, lba, off, hctx);
    }
    inline void Init(int ns_id, int op, int dev_id, char *user_buf, size_t lba, size_t off, int hctx) {
        header_.ns_id_ = ns_id;
        header_.op_ = op;
        dev_id_ = dev_id;
        user_buf_ = user_buf;
        lba_ = lba;
        off_ = off;
        hctx_ = hctx;
    }
#endif
};

struct labstor_complet_mq_driver_request {
    struct labstor_request header;
};

#endif //LABSTOR_REQUEST_LAYER_H
