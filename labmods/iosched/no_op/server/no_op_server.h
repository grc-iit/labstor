//
// Created by lukemartinlogan on 12/5/21.
//

#ifndef LABSTOR_NO_OP_IOSCHED_SERVER_H
#define LABSTOR_NO_OP_IOSCHED_SERVER_H

#include <labstor/userspace/server/server.h>
#include <labmods/storage_api/generic_block/generic_block.h>
#include <labmods/iosched/no_op/no_op.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/namespace.h>


namespace labstor::iosched::NoOp {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
    int next_module_, num_hw_queues_, queue_depth_;
public:
    Server() : labstor::Module(NO_OP_IOSCHED_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    bool ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
    void Initialize(labstor::ipc::request *rq);
    bool IO(labstor::queue_pair *qp, labstor::GenericBlock::io_request *client_rq, labstor::credentials *creds);
};

}

#endif //LABSTOR_NO_OP_IOSCHED_SERVER_H
