//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_IPC_TEST_CLIENT_H
#define LABSTOR_IPC_TEST_CLIENT_H

#include <labstor/userspace/client/client.h>
#include <labmods/ipc_test/ipc_test.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>

namespace labstor::IPCTest {

class Client: public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Client() : labstor::Module(IPC_TEST_MODULE_ID)  {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Initialize(int ns_id) {}
    void Register(YAML::Node config) override;
    int Start(int batch_size);
};

}

#endif //LABSTOR_IPC_TEST_CLIENT_H
