//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_DUMMY_SERVER_H
#define LABSTOR_DUMMY_SERVER_H

#include <labstor/userspace/server/server.h>
#include "dummy.h"
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>

namespace labstor::test::Dummy {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Server() : labstor::Module(LABSTOR_DUMMY_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Initialize(labstor::ipc::request *rq) {}
    void ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
};

}

#endif //LABSTOR_DUMMY_SERVER_H
