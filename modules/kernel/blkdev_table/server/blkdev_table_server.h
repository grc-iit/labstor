//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_DUMMY_SERVER_H
#define LABSTOR_DUMMY_SERVER_H

#include "blkdev_table.h"
#include <labstor/userspace/types/module.h>

#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>

namespace labstor::BlkdevTable {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Server() : labstor::Module(BLKDEV_TABLE_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    void ProcessRequest(labstor::ipc::queue_pair &qp, labstor::ipc::request *request, labstor::credentials *creds);
    void RegisterBlkdev(labstor::ipc::queue_pair &qp, labstor_submit_blkdev_table_register_request *rq_submit);
    void UnregisterBlkdev(labstor::ipc::queue_pair &qp, labstor_submit_blkdev_table_register_request *rq_submit);
};
LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Server)

}

#endif //LABSTOR_DUMMY_SERVER_H
