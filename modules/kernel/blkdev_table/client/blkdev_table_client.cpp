//
// Created by lukemartinlogan on 12/3/21.
//
#include <labstor/userspace/util/debug.h>
#include <modules/registrar/registrar.h>

#include "blkdev_table.h"
#include "blkdev_table_client.h"

void labstor::BlkdevTable::Client::Register() {
    AUTO_TRACE("labstor::BlkdevTable::Client")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance(BLKDEV_TABLE_MODULE_ID, BLKDEV_TABLE_MODULE_ID);
    TRACEPOINT("labstor::BlkdevTable::Client::Register::NamespaceID", ns_id_)
}

void labstor::BlkdevTable::Client::AddBdev(std::string path) {
    AUTO_TRACE("labstor::BlkdevTable::Client::AddBdev", ns_id_)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor_submit_blkdev_table_register_request *rq_submit;
    labstor_complete_blkdev_table_register_request *rq_complete;

    ipc_manager_->GetQueuePair(qp, 0);
    rq_submit = reinterpret_cast<labstor_submit_blkdev_table_register_request*>(
            ipc_manager_->AllocRequest(qp, labstor_submit_blkdev_table_register_request::GetSize(path.size())));
    rq_submit->Init(ns_id_, path.c_str(), path.size(), -1);

    TRACEPOINT("path", rq_submit->path_);
    qtok = qp->Enqueue(reinterpret_cast<labstor::ipc::request*>(rq_submit));
    TRACEPOINT("Enqueued");
    rq_complete = reinterpret_cast<labstor_complete_blkdev_table_register_request*>(ipc_manager_->Wait(qtok));
    ipc_manager_->FreeRequest(qtok, reinterpret_cast<labstor::ipc::request*>(rq_complete));
}

void labstor::BlkdevTable::Client::UnregisterBlkdev(int dev_id) {
    AUTO_TRACE("labstor::BlkdevTable::Client::UnregisterBlkdev", ns_id_)
}