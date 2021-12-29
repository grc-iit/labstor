//
// Created by lukemartinlogan on 12/5/21.
//

#ifndef LABSTOR_MQ_DRIVER_SERVER_H
#define LABSTOR_MQ_DRIVER_SERVER_H

#include <labstor/userspace/server/server.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/types/module.h>
#include "mq_driver.h"


namespace labstor::MQDriver {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Server() : labstor::Module(MQ_DRIVER_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
    void IOStart(labstor::ipc::queue_pair *qp, labstor_mq_driver_request *rq_submit, labstor::credentials *creds);
    void IOComplete(labstor::ipc::queue_pair *private_qp, labstor_mq_driver_poll_request *poll_rq);
};

}

#endif //LABSTOR_MQ_DRIVER_SERVER_H
