//
// Created by lukemartinlogan on 11/26/21.
//

#include "blkdev_table_server.h"
#include <labstor/constants/constants.h>

void labstor::BlkdevTable::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("labstor::BlkdevTable::ProcessRequest", request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kRegisterBdev: {
            RegisterBlkdev(qp, reinterpret_cast<labstor_submit_blkdev_table_register_request *>(request), creds);
            break;
        }
        case Ops::kRegisterBdevComplete: {
            RegisterBlkdevComplete(qp, reinterpret_cast<labstor_poll_blkdev_table_register*>(request));
            break;
        }
        case Ops::kUnregisterBdev: {
            UnregisterBlkdev(qp, reinterpret_cast<labstor_submit_blkdev_table_register_request *>(request));
            break;
        }
    }
}

void labstor::BlkdevTable::Server::RegisterBlkdev(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq_submit, labstor::credentials *creds) {
    AUTO_TRACE("labstor::BlkdevTable::Server::RegisterBlkdev", "qp_ptr", (size_t)qp, "qp_id", qp->GetQid(), "path", rq_submit->path_);
    labstor_poll_blkdev_table_register *poll_rq;
    labstor_submit_blkdev_table_register_request *kern_submit;
    labstor::ipc::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL & PRIVATE QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
       LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
       KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create SERVER -> KERNEL message
    kern_submit = ipc_manager_->AllocRequest<labstor_submit_blkdev_table_register_request>(kern_qp);
    kern_submit->Init(BLKDEV_TABLE_RUNTIME_ID, rq_submit);
    dev_ids_.Dequeue(kern_submit->dev_id_);
    TRACEPOINT("labstor::BlkdevTable::Server::RegisterBlkdev", "KERN SUBMIT", kern_submit->path_);
    qtok = kern_qp->Enqueue<labstor_submit_blkdev_table_register_request>(kern_submit);

    //Poll SERVER -> KERNEL interaction
    TRACEPOINT("labstor::BlkdevTable::Server::RegisterBlkdev", "Allocating Poll RQ", "private_qp_id", private_qp->GetQid())
    poll_rq = ipc_manager_->AllocRequest<labstor_poll_blkdev_table_register>(private_qp);
    poll_rq->Init(qp, rq_submit, qtok);
    private_qp->Enqueue<labstor_poll_blkdev_table_register>(poll_rq);

    //Release requests
    TRACEPOINT("labstor::BlkdevTable::Server::RegisterBlkdev",
               "rq_submit",
               (size_t)rq_submit - (size_t)ipc_manager_->GetRegion(creds->pid),
               "kern_submit",
               (size_t)kern_submit - (size_t)ipc_manager_->GetRegion(KERNEL_PID));
    ipc_manager_->FreeRequest<labstor_submit_blkdev_table_register_request>(qp, rq_submit);
}

void labstor::BlkdevTable::Server::RegisterBlkdevComplete(labstor::ipc::queue_pair *private_qp, labstor_poll_blkdev_table_register *poll_rq) {
    AUTO_TRACE("labstor::BlkdevTable::Server::RegisterBlkdevComplete");
    labstor::ipc::queue_pair *qp, *kern_qp;
    labstor_complete_blkdev_table_register_request *kern_complete;
    labstor_complete_blkdev_table_register_request *rq_complete;

    //Check if the QTOK has been completed
    TRACEPOINT("labstor::BlkdevTable::Server::RegisterBlkdevComplete", "Check if I/O has completed")
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->kqtok_);
    if(!kern_qp->IsComplete<labstor_complete_blkdev_table_register_request>(poll_rq->kqtok_, kern_complete)) {
        private_qp->Enqueue<labstor_poll_blkdev_table_register>(poll_rq);
        return;
    }

    //Create message for the USER
    TRACEPOINT("labstor::BlkdevTable::Server::RegisterBlkdevComplete", "Create message for user", poll_rq->uqtok_.req_id)
    ipc_manager_->GetQueuePair(qp, poll_rq->uqtok_);
    rq_complete = ipc_manager_->AllocRequest<labstor_complete_blkdev_table_register_request>(qp);
    rq_complete->header_.op_ = kern_complete->header_.op_;

    //Complete SERVER -> USER interaction
    TRACEPOINT("labstor::BlkdevTable::Server::RegisterBlkdevComplete", "request complete")
    qp->Complete<labstor_complete_blkdev_table_register_request>(poll_rq->uqtok_, rq_complete);

    //Free completed requests
    TRACEPOINT("labstor::BlkdevTable::Server::RegisterBlkdevComplete", "Free",
               "kern_complete",
               (size_t)kern_complete - (size_t)ipc_manager_->GetRegion(KERNEL_PID),
               (int)rq_complete->header_.op_);
    ipc_manager_->FreeRequest<labstor_complete_blkdev_table_register_request>(kern_qp, kern_complete);
    ipc_manager_->FreeRequest<labstor_poll_blkdev_table_register>(private_qp, poll_rq);
}

void labstor::BlkdevTable::Server::UnregisterBlkdev(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq_submit) {
    AUTO_TRACE("labstor::BlkdevTable::RemoveBdev")
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Server)