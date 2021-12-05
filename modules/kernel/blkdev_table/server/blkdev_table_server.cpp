//
// Created by lukemartinlogan on 11/26/21.
//

#include "blkdev_table_server.h"
#include <labstor/constants/constants.h>

void labstor::BlkdevTable::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("labstor::BlkdevTable::ProcessRequest", request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kRegisterBdev: {
            RegisterBlkdev(qp, reinterpret_cast<labstor_submit_blkdev_table_register_request *>(request));
            break;
        }
        case Ops::kUnregisterBdev: {
            UnregisterBlkdev(qp, reinterpret_cast<labstor_submit_blkdev_table_register_request *>(request));
            break;
        }
    }
}

void labstor::BlkdevTable::Server::RegisterBlkdev(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq_submit) {
    AUTO_TRACE("labstor::BlkdevTable::AddBdev", rq_submit->path_);
    labstor_submit_blkdev_table_register_request *kern_submit;
    labstor_complete_blkdev_table_register_request *kern_complete;
    labstor::ipc::queue_pair *kern_qp;
    labstor::ipc::qtok_t qtok;

    //Get kernel QP
    ipc_manager_->GetQueuePair(kern_qp,
       LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
       0, KERNEL_PID);

    //Create request
    kern_submit = reinterpret_cast<labstor_submit_blkdev_table_register_request *>(
            ipc_manager_->AllocRequest(kern_qp, sizeof(labstor_submit_blkdev_table_register_request)));
    kern_submit->Init(BLKDEV_TABLE_RUNTIME_ID, rq_submit);
    dev_ids_.Dequeue(kern_submit->dev_id_);
    TRACEPOINT("KERN SUBMIT", kern_submit->path_);

    //Send message to kernel
    qtok = kern_qp->Enqueue(reinterpret_cast<labstor::ipc::request*>(kern_submit));
    kern_complete = reinterpret_cast<labstor_complete_blkdev_table_register_request *>(ipc_manager_->Wait(qtok));

    //Complete request
    TRACEPOINT("labstor::BlkdevTable::AddBdev", "Kernel Complete", (size_t)kern_complete, kern_complete->header.ns_id_);
    qp->Complete(
            reinterpret_cast<labstor::ipc::request*>(rq_submit),
            reinterpret_cast<labstor::ipc::request*>(kern_complete));
}

void labstor::BlkdevTable::Server::UnregisterBlkdev(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq_submit) {
    AUTO_TRACE("labstor::BlkdevTable::RemoveBdev")
}