//
// Created by lukemartinlogan on 12/5/21.
//

#ifndef LABSTOR_MQ_DRIVER_SERVER_H
#define LABSTOR_MQ_DRIVER_SERVER_H

#include "blkdev_table.h"

namespace labstor::MQDriver {

class Server {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Server() : labstor::Module(MQ_DRIVER_MODULE) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
    void IO(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq_submit);
};

}

#endif //LABSTOR_MQ_DRIVER_SERVER_H
