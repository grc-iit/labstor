//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_DUMMY_SERVER_H
#define LABSTOR_DUMMY_SERVER_H

#include "dummy.h"
#include <labstor/types/module.h>

#include <labstor/userspace_server/macros.h>
#include <labstor/userspace_server/ipc_manager.h>

namespace labstor::test::Dummy {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Server() : labstor::Module("Dummy") {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    void ProcessRequest(labstor::ipc::queue_pair &qp, labstor::ipc::request *request, labstor::credentials *creds);
};
LABSTOR_MODULE_CONSTRUCT(labstor::test::Dummy::Server)

}

#endif //LABSTOR_DUMMY_SERVER_H
