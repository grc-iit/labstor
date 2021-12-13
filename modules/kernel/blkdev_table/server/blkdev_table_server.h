//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_BLKDEV_TABLE_SERVER_H
#define LABSTOR_BLKDEV_TABLE_SERVER_H

#include <labstor/userspace/server/server.h>

#include "blkdev_table.h"
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>

#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_uint32_t.h>

namespace labstor::BlkdevTable {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    labstor::ipc::ring_buffer_uint32_t dev_ids_;
public:
    Server() : labstor::Module(BLKDEV_TABLE_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        uint32_t region_size = labstor::ipc::ring_buffer_uint32_t::GetSize(MAX_MOUNTED_BDEVS);
        void *region = malloc(region_size);
        dev_ids_.Init(region, region_size);
        for(int i = 0; i < MAX_MOUNTED_BDEVS; ++i) {
            dev_ids_.Enqueue(i);
        }
    }

    void ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
    void RegisterBlkdev(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq_submit, labstor::credentials *creds);
    void RegisterBlkdevComplete(labstor::ipc::queue_pair *private_qp, labstor_poll_blkdev_table_register *poll_rq);
    void UnregisterBlkdev(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq_submit);
};

}

#endif //LABSTOR_BLKDEV_TABLE_SERVER_H
