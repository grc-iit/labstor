//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_IOSCHED_MANUAL_CLIENT_H
#define LABSTOR_IOSCHED_MANUAL_CLIENT_H

#include <labstor/userspace/types/module.h>
#include <labstor/userspace/types/messages.h>
#include <labstor/constants/macros.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>
#include <kernel/mq_driver/request_layer.h>

namespace labstor::iosched::Manual {

class Server: public labstor::Module {
private:
    LABSTOR_NAMESPACE_T namespace_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    int ns_id_;
    int dev_id_;
public:
    Server() {
        namespace_ = LABSTOR_NAMESPACE;
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    void ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
        switch(request->op) {
            case labstor::iosched::ManualOps::kWrite: {
                break;
            }
            case labstor::iosched::ManualOps::kRead: {
                break;
            }
            case labstor::iosched::ManualOps::kPoll: {
                break;
            }
        }
    }

    void Register(std::string dev_name) {
        namespace_.AddKey(dev_name, CreateModule());
    }

    inline void IO(void *buf, size_t buf_size, size_t lba, int hctx, labstor::ipc::driver::MQOps op) {
        labstor::ipc::queue_pair kernel_qp;
        mq_submit_request *mq_rq = ipc_manager_->AllocRequest<mq_driver_request>(sizeof(labstor::ipc::poll_request));
        ipc_manager_->GetQueuePair(kernel_qp, LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_STREAM | LABSTOR_QP_LOW_LATENCY, 0, KERNEL_PID);
        mq_rq->op_ = op;
        mq_rq->buf_ = buf;
        mq_rq->buf_size_ = buf_size;
        mq_rq->lba_ = lba;
        mq_rq->hctx_ = hctx;
        qtok_t qtok = kernel_qp->sq.Enqueue(rq, qtok);

        labstor::ipc::queue_pair poll_qp;
        ipc_manager_->GetQueuePair(poll_qp, LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_STREAM | LABSTOR_QP_LOW_LATENCY);
        labstor::ipc::poll_request *poll_rq = (labstor::ipc::poll_request*)ipc_manager_->Alloc(sizeof(labstor::ipc::poll_request));
        poll_rq->ns_id_ = ns_id_;
        poll_rq->op_ = kPoll;
        poll_rq->req_id_ = qtok;
        poll_qp->sq.Enqueue(poll_rq);
    }

    void Write(void *buf, size_t buf_size, size_t lba, int hctx) {
        IO(buf, buf_size, lba, hctx, labstor::ipc::driver::MQOps::kWrite);
    }

    void Read(void *buf, size_t buf_size, size_t lba, int hctx) {
        IO(buf, buf_size, lba, hctx, labstor::ipc::driver::MQOps::kRead);
    }

    void PollRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request) {
        labstor::ipc::poll_request new_poll_rq = (labstor::ipc::poll_request*)ipc_manager_->Alloc(sizeof(labstor::ipc::poll_request));
        *new_poll_rq = *((labstor:ipc::poll_request*)request);
        qp->sq.Enqueue(new_poll_rq);
    }
};

}

#endif //LABSTOR_IOSCHED_MANUAL_CLIENT_H
