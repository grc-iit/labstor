//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_IPC_TEST_SERVER_H
#define LABSTOR_IPC_TEST_SERVER_H

#include "ipc_test.h"
#include <labstor/userspace/types/module.h>

#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>

#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_uint32_t.h>

namespace labstor::IPCTest {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Server() : labstor::Module(IPC_TEST_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
    void Start(labstor::ipc::queue_pair *qp, labstor_submit_ipc_test_request *rq_submit);
    void End(labstor::ipc::queue_pair *qp, labstor_poll_ipc_test_request *rq_submit);
};

}

#endif //LABSTOR_IPC_TEST_SERVER_H
