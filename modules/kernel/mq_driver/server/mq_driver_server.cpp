//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "mq_driver.h"
#include "mq_driver_server.h"

void labstor::MQDriver::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("labstor::MQDriver::Server::ProcessRequest", request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kWrite:
        case Ops::kRead: {
            IOStart(qp, reinterpret_cast<labstor_submit_mq_driver_request*>(request), creds);
            break;
        }
        case Ops::kIOComplete: {
            IOComplete(qp, reinterpret_cast<labstor_mq_driver_poll_request*>(request));
            break;
        }
    }
}

void labstor::MQDriver::Server::IOStart(labstor::ipc::queue_pair *qp, labstor_submit_mq_driver_request *rq_submit, labstor::credentials *creds) {
    AUTO_TRACE("labstor::MQDriver::Server::IOStart", rq_submit->dev_id_);
    labstor_mq_driver_poll_request *poll_rq;
    labstor_submit_mq_driver_request *kern_submit;
    labstor::ipc::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                               KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_qp,
                               LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create SERVER -> KERNEL message
    kern_submit = ipc_manager_->AllocRequest<labstor_submit_mq_driver_request>(kern_qp);
    kern_submit->Init(MQ_DRIVER_RUNTIME_ID, rq_submit);
    kern_qp->Enqueue<labstor_submit_mq_driver_request>(kern_submit, qtok);

    //Poll SERVER -> KERNEL interaction
    poll_rq = ipc_manager_->AllocRequest<labstor_mq_driver_poll_request>(private_qp);
    poll_rq->Init(qp, rq_submit, qtok);
    private_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);

    //Free used requests
    TRACEPOINT("labstor::MQDriver::Server::IO",
               "rq_submit",
               (size_t)rq_submit - (size_t)ipc_manager_->GetRegion(creds->pid),
               "kern_submit",
               (size_t)kern_submit - (size_t)ipc_manager_->GetRegion(KERNEL_PID));
    ipc_manager_->FreeRequest<labstor_submit_mq_driver_request>(qp, rq_submit);
}

void labstor::MQDriver::Server::IOComplete(labstor::ipc::queue_pair *private_qp, labstor_mq_driver_poll_request *poll_rq) {
    AUTO_TRACE("labstor::MQDriver::Server::IOComplete", poll_rq->header_.op_);
    labstor::ipc::queue_pair *qp, *kern_qp;
    labstor_complete_mq_driver_request *kern_complete;
    labstor_complete_mq_driver_request *rq_complete;
    labstor::ipc::qtok_t qtok;

    //Check if the QTOK has been completed
    TRACEPOINT("labstor::MQDriver::Server::IOComplete", "Check if I/O has completed")
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->kqtok_);
    if(!kern_qp->IsComplete<labstor_complete_mq_driver_request>(poll_rq->kqtok_, kern_complete)) {
        private_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }

    //Create message for the USER
    TRACEPOINT("labstor::MQDriver::Server::IOComplete", "Create message for user")
    ipc_manager_->GetQueuePair(qp, poll_rq->uqtok_);
    rq_complete = ipc_manager_->AllocRequest<labstor_complete_mq_driver_request>(qp);
    rq_complete->header_.op_ = kern_complete->header_.op_;

    //Complete SERVER -> USER interaction
    TRACEPOINT("labstor::MQDriver::Server::IOComplete", "request complete!")
    qp->Complete<labstor_complete_mq_driver_request>(poll_rq->uqtok_, rq_complete);

    //Free completed requests
    TRACEPOINT("labstor::MQDriver::Server::IO", "Complete",
               "kern_complete",
               (size_t)kern_complete - (size_t)ipc_manager_->GetRegion(KERNEL_PID),
               (int)rq_complete->header_.op_);
    ipc_manager_->FreeRequest<labstor_complete_mq_driver_request>(kern_qp, kern_complete);
    ipc_manager_->FreeRequest<labstor_mq_driver_poll_request>(private_qp, poll_rq);
}

LABSTOR_MODULE_CONSTRUCT(labstor::MQDriver::Server);