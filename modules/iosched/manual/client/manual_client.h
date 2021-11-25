//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_IOSCHED_MANUAL_CLIENT_H
#define LABSTOR_IOSCHED_MANUAL_CLIENT_H

#include <labstor/types/module.h>
#include <labstor/constants/macros.h>
#include <labstor/userspace_client/macros.h>
#include <labstor/userspace_client/ipc_manager.h>
#include <labstor/userspace_client/namespace.h>

namespace labstor::Client::iosched {

class Manual : public labstor::Module {
private:
    LABSTOR_NAMESPACE_T namespace_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    int ns_id_;
    int dev_id_;
public:
    Manual() {
        namespace_ = LABSTOR_NAMESPACE;
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    void Init(std::string dev_name) {
        ns_id_ = namespace_.GetNamespaceID(dev_name);
        dev_id_ = dev_id;
    }
    void Write(void *buf, size_t buf_size, size_t lba, int hctx) {
        labstor::ipc::qtok_t qtok;
        labstor::ipc::queue_pair qp;
        ipc_manager_->GetQueuePair(qp, LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);
        qtok = qp.Enqueue();
        qp.Wait(qtok);
    }
    void Read(void *buf, size_t buf_size, size_t lba, int hctx) {
    }
};

}

#endif //LABSTOR_IOSCHED_MANUAL_CLIENT_H
