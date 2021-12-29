//
// Created by lukemartinlogan on 11/26/21.
//

#include "blkdev_table_server.h"
#include <labstor/constants/constants.h>

void labstor::BlkdevTable::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE(request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kRegisterBdev: {
            RegisterBlkdev(qp, reinterpret_cast<labstor_blkdev_table_register_request *>(request), creds);
            break;
        }
        case Ops::kRegisterBdevComplete: {
            RegisterBlkdevComplete(qp, reinterpret_cast<labstor_poll_blkdev_table_register*>(request));
            break;
        }
        case Ops::kUnregisterBdev: {
            UnregisterBlkdev(qp, reinterpret_cast<labstor_blkdev_table_register_request *>(request));
            break;
        }
    }
}

void labstor::BlkdevTable::Server::RegisterBlkdev(labstor::ipc::queue_pair *qp, labstor_blkdev_table_register_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE("qp_ptr", (size_t)qp, "qp_id", qp->GetQid(), "path", client_rq->path_);
    labstor_poll_blkdev_table_register *poll_rq;
    labstor_blkdev_table_register_request *kern_rq;
    labstor::ipc::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL & PRIVATE QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
       LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
       KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create SERVER -> KERNEL message
    kern_rq = ipc_manager_->AllocRequest<labstor_blkdev_table_register_request>(kern_qp);
    kern_rq->Start(BLKDEV_TABLE_RUNTIME_ID, client_rq);
    if(!dev_ids_.Dequeue(kern_rq->dev_id_)) {
        //TODO; reply error to user
    }
    TRACEPOINT("KERN SUBMIT", kern_rq->path_, "dev_id", kern_rq->dev_id_);
    kern_qp->Enqueue<labstor_blkdev_table_register_request>(kern_rq, qtok);

    //Poll SERVER -> KERNEL interaction
    TRACEPOINT("Allocating Poll RQ", "private_qp_id", private_qp->GetQid())
    poll_rq = ipc_manager_->AllocRequest<labstor_poll_blkdev_table_register>(private_qp);
    poll_rq->Init(qp, client_rq, qtok);
    private_qp->Enqueue<labstor_poll_blkdev_table_register>(poll_rq, qtok);
}

void labstor::BlkdevTable::Server::RegisterBlkdevComplete(labstor::ipc::queue_pair *private_qp, labstor_poll_blkdev_table_register *poll_rq) {
    AUTO_TRACE("");
    labstor::ipc::queue_pair *qp, *kern_qp;
    labstor_blkdev_table_register_request *kern_rq;
    labstor_blkdev_table_register_request *client_rq;
    labstor::ipc::qtok_t qtok;

    //Check if the QTOK has been completed
    TRACEPOINT("Check if I/O has completed")
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->poll_qtok_);
    if(!kern_qp->IsComplete<labstor_blkdev_table_register_request>(poll_rq->poll_qtok_, kern_rq)) {
        private_qp->Enqueue<labstor_poll_blkdev_table_register>(poll_rq, qtok);
        return;
    }

    //Create message for the USER
    TRACEPOINT("Device id", kern_rq->GetDeviceID())
    ipc_manager_->GetQueuePair(qp, poll_rq->reply_qtok_);
    client_rq = poll_rq->reply_rq_;
    client_rq->Copy(kern_rq);

    //Complete SERVER -> USER interaction
    TRACEPOINT("request complete")
    qp->Complete<labstor_blkdev_table_register_request>(poll_rq->reply_qtok_, client_rq);

    //Free completed requests
    ipc_manager_->FreeRequest<labstor_blkdev_table_register_request>(kern_qp, kern_rq);
    ipc_manager_->FreeRequest<labstor_poll_blkdev_table_register>(private_qp, poll_rq);
}

void labstor::BlkdevTable::Server::UnregisterBlkdev(labstor::ipc::queue_pair *qp, labstor_blkdev_table_register_request *client_rq) {
    AUTO_TRACE("")
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Server)