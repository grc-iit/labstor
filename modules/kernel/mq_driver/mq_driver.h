//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_MQ_DRIVER_H
#define LABSTOR_MQ_DRIVER_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>

#define MQ_DRIVER_MODULE_ID "MQ_DRIVER"

#ifdef __cplusplus
namespace labstor::MQDriver {
enum class Ops {
    kRegister,
    kWrite,
    kRead,
    kIOComplete
};
}
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
    void *user_buf_;
    size_t sector_;
    size_t buf_size_;
    int hctx_;
    int pid_;

#ifdef __cplusplus
    inline void Init(int ns_id, int pid, labstor::MQDriver::Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        Init(ns_id, pid, static_cast<int>(op), dev_id, user_buf, buf_size, sector, hctx);
    }
    inline void Init(int ns_id, struct labstor_submit_mq_driver_request *rq) {
        Init(ns_id, rq->pid_, rq->header_.op_, rq->dev_id_, rq->user_buf_, rq->buf_size_, rq->sector_, rq->hctx_);
    }
    inline void Init(int ns_id, int pid, int op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        header_.ns_id_ = ns_id;
        header_.op_ = op;
        pid_ = pid;
        dev_id_ = dev_id;
        user_buf_ = user_buf;
        buf_size_ = buf_size;
        sector_ = sector;
        hctx_ = hctx;
    }
#endif

    struct labstor_queue_pair *qp_;
};

struct labstor_mq_driver_poll_request {
    struct labstor_request header_;
    struct labstor_qtok_t kqtok_;
    struct labstor_qtok_t uqtok_;
#ifdef __cplusplus
    void Init(labstor_queue_pair *qp, labstor_submit_mq_driver_request *rq, labstor::ipc::qtok_t &qtok) {
        header_.ns_id_ = rq->header_.ns_id_;
        header_.op_ = static_cast<int>(labstor::MQDriver::Ops::kIOComplete);
        kqtok_ = qtok;
        uqtok_.qid = qp->GetQid();
        uqtok_.req_id = rq->header_.req_id_;
    }
#endif
};

struct labstor_complete_mq_driver_request {
    struct labstor_request header_;
};

#endif //LABSTOR_MQ_DRIVER_H
