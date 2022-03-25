//
// Created by lukemartinlogan on 12/5/21.
//

#ifndef LABSTOR_MQ_DRIVER_CLIENT_H
#define LABSTOR_MQ_DRIVER_CLIENT_H

#include <labstor/userspace/client/client.h>
#include <labmods/iosched/no_op/no_op.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>

namespace labstor::iosched::NoOp {

class Client: public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    uint32_t ns_id_;
public:
    Client() : labstor::Module(NO_OP_IOSCHED_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Register(const std::string &ns_key, const std::string &dev_name);
    void Initialize(labstor::ipc::request *rq) {}
    int GetNamespaceID();
};

}

#endif //LABSTOR_MQ_DRIVER_CLIENT_H
