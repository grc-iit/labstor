//
// Created by lukemartinlogan on 11/26/21.
//

#include "blkdev_table_server.h"
#include <labstor/constants/constants.h>

bool labstor::BlkdevTable::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE(request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kRegisterBdev: {
            return RegisterBlkdev(qp, reinterpret_cast<blkdev_table_register_request*>(request), creds);
        }
    }
    return false;
}

bool labstor::BlkdevTable::Server::RegisterBlkdev(labstor::queue_pair *qp, blkdev_table_register_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE("qp_ptr", (size_t)qp, "qp_id", qp->GetQID().Hash(), "path", client_rq->path_);
    blkdev_table_register_request *kern_rq;
    labstor::queue_pair *kern_qp;

    //Get KERNEL & PRIVATE QP
    switch(client_rq->GetCode()) {
        case 0: {
            ipc_manager_->GetQueuePairByPid(kern_qp,
                                            LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE |
                                            LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                            KERNEL_PID);
            //Create SERVER -> KERNEL message
            kern_rq = ipc_manager_->AllocRequest<blkdev_table_register_request>(kern_qp);
            kern_rq->ServerStart(BLKDEV_TABLE_RUNTIME_ID, client_rq);
            if (!dev_ids_.Dequeue(kern_rq->dev_id_)) {
                //TODO; reply error to user and free
            }
            TRACEPOINT("KERN SUBMIT", kern_rq->path_, "dev_id", kern_rq->dev_id_);
            kern_qp->Enqueue<blkdev_table_register_request>(kern_rq, client_rq->kern_qtok_);
            client_rq->SetCode(1);
            return false;
        }
        case 1: {
            ipc_manager_->GetQueuePair(kern_qp, client_rq->kern_qtok_);
            if(!kern_qp->IsComplete<blkdev_table_register_request>(client_rq->kern_qtok_, kern_rq)) {
                return false;
            }
            client_rq->Copy(kern_rq);
            qp->Complete<blkdev_table_register_request>(client_rq);
            ipc_manager_->FreeRequest<blkdev_table_register_request>(kern_qp, kern_rq);
            return true;
        }
    }
    return true;
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Server, BLKDEV_TABLE_MODULE_ID)