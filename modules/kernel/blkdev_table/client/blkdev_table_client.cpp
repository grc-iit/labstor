//
// Created by lukemartinlogan on 12/3/21.
//
#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "blkdev_table.h"
#include "blkdev_table_client.h"

void labstor::BlkdevTable::Client::Register() {
    AUTO_TRACE("labstor::BlkdevTable::Client")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance(BLKDEV_TABLE_MODULE_ID, BLKDEV_TABLE_MODULE_ID);
    TRACEPOINT("labstor::BlkdevTable::Client::Register::NamespaceID", ns_id_)
}

int labstor::BlkdevTable::Client::RegisterBlkdev(std::string path) {
    AUTO_TRACE("labstor::BlkdevTable::Client::AddBdev", ns_id_)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor_submit_blkdev_table_register_request *rq_submit;
    labstor_complete_blkdev_table_register_request *rq_complete;
    int dev_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq_submit = ipc_manager_->AllocRequest<labstor_submit_blkdev_table_register_request>(qp,
         labstor_submit_blkdev_table_register_request::GetSize(path.size()));
    rq_submit->Init(ns_id_, path.c_str(), path.size(), -1);

    TRACEPOINT("labstor::BlkdevTable::Client::IO", "path", rq_submit->path_, "qp_id", qp->GetQid());
    qp->Enqueue<labstor_submit_blkdev_table_register_request>(rq_submit, qtok);
    rq_complete = ipc_manager_->Wait<labstor_complete_blkdev_table_register_request>(qtok);
    dev_id = rq_complete->GetDeviceID();
    TRACEPOINT("labstor::BlkdevTable::Client::IO", "Complete",
               "return_code", rq_complete->GetReturnCode(),
               "device_id", rq_complete->GetDeviceID());
    TRACEPOINT("labstor::BlkdevTable::Client::IO", "Free",
               "rq_complete",
               (size_t)rq_complete - (size_t)ipc_manager_->GetBaseRegion());
    ipc_manager_->FreeRequest<labstor_complete_blkdev_table_register_request>(qtok, rq_complete);
    return dev_id;
}

void labstor::BlkdevTable::Client::UnregisterBlkdev(int dev_id) {
    AUTO_TRACE("labstor::BlkdevTable::Client::UnregisterBlkdev", ns_id_)
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Client)