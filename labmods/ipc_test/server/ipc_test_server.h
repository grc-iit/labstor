//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_IPC_TEST_SERVER_H
#define LABSTOR_IPC_TEST_SERVER_H

#include <labstor/userspace/server/server.h>

#include "ipc_test.h"
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>

#include "labstor/types/data_structures/shmem_ring_buffer.h"

namespace labstor::IPCTest {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Server() : labstor::Module(IPC_TEST_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    bool ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
    bool Initialize(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) { return true; }
    bool IPC(labstor::queue_pair *qp, ipc_test_request *rq);
};

}

#endif //LABSTOR_IPC_TEST_SERVER_H
