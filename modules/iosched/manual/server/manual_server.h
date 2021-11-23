//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_MANUAL_SERVER_H
#define LABSTOR_MANUAL_SERVER_H

#include <labstor/types/module.h>
#include <labstor/constants/macros.h>
#include <labstor/userspace_server/macros.h>
#include <labstor/userspace_server/ipc_manager.h>
#include <labstor/userspace_server/namespace.h>

namespace labstor::iosched::Server {

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

    void ProcessRequest(labstor::ipc::queue_pair &qp, labstor::ipc::request *request, labstor::credentials *creds) {
        labstor::ipc::queue_pair qp2 = ipc_manager_->GetQueuePair(LABSTOR_QP_INTERMEDIATE);
    }

    void Init(std::string dev_name) {
    }
    void Write(void *buf, size_t buf_size, size_t lba, int hctx) {
    }
    void Read(void *buf, size_t buf_size, size_t lba, int hctx);
};

}

#endif //LABSTOR_MANUAL_SERVER_H
