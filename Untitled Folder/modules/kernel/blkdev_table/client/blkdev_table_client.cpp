//
// Created by lukemartinlogan on 12/3/21.
//
#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "blkdev_table.h"
#include "blkdev_table_client.h"

void labstor::BlkdevTable::Client::Register() {
    AUTO_TRACE("")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance(BLKDEV_TABLE_MODULE_ID, BLKDEV_TABLE_MODULE_ID);
    TRACEPOINT(ns_id_)
}

int labstor::BlkdevTable::Client::RegisterBlkdev(std::string path) {
    AUTO_TRACE(ns_id_)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor_blkdev_table_register_request *rq;
    int dev_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<labstor_blkdev_table_register_request>(qp,
         labstor_blkdev_table_register_request::GetSize(path.size()));
    rq->Start(ns_id_, path.c_str(), path.size(), -1);

    TRACEPOINT("path", rq->path_, "qp_id", qp->GetQid());
    qp->Enqueue<labstor_blkdev_table_register_request>(rq, qtok);
    rq = ipc_manager_->Wait<labstor_blkdev_table_register_request>(qtok);
    dev_id = rq->GetDeviceID();
    TRACEPOINT("Complete",
               "return_code", rq->GetReturnCode(),
               "device_id", rq->GetDeviceID());
    ipc_manager_->FreeRequest<labstor_blkdev_table_register_request>(qtok, rq);
    return dev_id;
}

void labstor::BlkdevTable::Client::UnregisterBlkdev(int dev_id) {
    AUTO_TRACE(ns_id_)
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Client)