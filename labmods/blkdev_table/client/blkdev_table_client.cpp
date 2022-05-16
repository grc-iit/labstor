//
// Created by lukemartinlogan on 12/3/21.
//
#include <labstor/constants/debug.h>
#include <labmods/registrar/registrar.h>

#include "blkdev_table.h"
#include "blkdev_table_client.h"

void labstor::BlkdevTable::Client::Register(YAML::Node config) {
    AUTO_TRACE("")
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(BLKDEV_TABLE_MODULE_ID, BLKDEV_TABLE_MODULE_ID);
    TRACEPOINT(ns_id_)
}

int labstor::BlkdevTable::Client::RegisterBlkdev(std::string path) {
    AUTO_TRACE(ns_id_)
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    blkdev_table_register_request *rq;
    int dev_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<blkdev_table_register_request>(qp,
         blkdev_table_register_request::GetSize(path.size()));
    rq->ClientStart(ns_id_, path.c_str(), path.size(), -1);

    TRACEPOINT("path", rq->path_, "qp_id", qp->GetQID().Hash());
    qp->Enqueue<blkdev_table_register_request>(rq, qtok);
    rq = ipc_manager_->Wait<blkdev_table_register_request>(qtok);
    dev_id = rq->GetDeviceID();
    TRACEPOINT("Complete",
               "return_code", rq->GetCode(),
               "device_id", rq->GetDeviceID());
    ipc_manager_->FreeRequest<blkdev_table_register_request>(qtok, rq);
    return dev_id;
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Client, BLKDEV_TABLE_MODULE_ID)