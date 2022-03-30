//
// Created by lukemartinlogan on 12/5/21.
//

#ifndef LABSTOR_NO_OP_IOSCHED_CLIENT_H
#define LABSTOR_NO_OP_IOSCHED_CLIENT_H

#include <labstor/userspace/client/client.h>
#include <labmods/iosched/no_op/no_op.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>
#include <labmods/storage_api/generic_block/client/generic_block_client.h>

namespace labstor::iosched::NoOp {

class Client: public labstor::GenericBlock::Client {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Client() : labstor::GenericBlock::Client(NO_OP_IOSCHED_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    labstor::ipc::qtok_t AIO(void *buf, size_t size, size_t off, labstor::GenericBlock::Ops op) override;
    void Register(const std::string &ns_key, const std::string &dev_name);
    void Initialize(labstor::ipc::request *rq) {}
    int GetNamespaceID();
};

}

#endif //LABSTOR_NO_OP_IOSCHED_CLIENT_H
