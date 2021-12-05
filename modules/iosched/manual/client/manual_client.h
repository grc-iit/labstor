//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_IOSCHED_MANUAL_CLIENT_H
#define LABSTOR_IOSCHED_MANUAL_CLIENT_H

#include <labstor/userspace/types/module.h>
#include <labstor/constants/macros.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>

namespace labstor::iosched::Manual {

class Client : public labstor::Module {
private:
    LABSTOR_NAMESPACE_T namespace_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    int ns_id_;
public:
    Client() {
        namespace_ = LABSTOR_NAMESPACE;
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    void Init(std::string dev_name) {
        ns_id_ = namespace_.GetNamespaceID(dev_name);
    }

    labstor::ipc::qtok AWrite(void *buf, size_t buf_size, size_t lba, int hctx);
    labstor::ipc::qtok ARead(void *buf, size_t buf_size, size_t lba, int hctx);
    void Write(void *buf, size_t buf_size, size_t lba, int hctx);
    void Read(void *buf, size_t buf_size, size_t lba, int hctx);

    inline labstor::ipc::qtok_t AIO(ManualOp op, void *buf, size_t buf_size, size_t lba, int hctx) {
        labstor::ipc::qtok_t qtok;
        labstor::ipc::queue_pair *qp;
        mq_submit_request *io_start;

        ipc_manager_->GetQueuePair(qp, LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);
        io_start = ipc_manager_->AllocRequest(qp, sizeof(io_request));
        io_start->Init(ns_id_, op, buf, buf_size, lba, hctx);
        qtok = qp->Enqueue(io_start);
        return qtok;
    }

    inline void IO(ManualOp op, void *buf, size_t buf_size, size_t lba, int hctx) {
        labstor::ipc::qtok_t qtok;
        mq_complete_request *io_end;
        qtok = AIO(op, buf, buf_size, lba, hctx);
        io_end = ipc_manager_->Wait(qtok);
        ipc_manager_->FreeRequest(qtok, io_end);
    }
};

}

#endif //LABSTOR_IOSCHED_MANUAL_CLIENT_H
