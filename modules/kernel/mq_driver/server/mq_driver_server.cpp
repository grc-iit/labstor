//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/userspace/util/debug.h>
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
            IOComplete(qp, reinterpret_cast<labstor_complete_mq_driver_request*>(request));
            break;
        }
    }
}

void labstor::MQDriver::Server::IOStart(labstor::ipc::queue_pair *qp, labstor_submit_mq_driver_request *rq_submit, labstor::credentials *creds) {
    AUTO_TRACE("labstor::MQDriver::Server::IOStart", rq_submit->dev_id_);
    labstor_complete_mq_driver_request *rq_complete;
    labstor_submit_mq_driver_request *kern_submit;
    labstor_complete_mq_driver_request *kern_complete;
    labstor::ipc::queue_pair *kern_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL QP
    ipc_manager_->GetQueuePair(kern_qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                               0, KERNEL_PID);

    //Create SERVER -> KERNEL message
    kern_submit = reinterpret_cast<labstor_submit_mq_driver_request*>(
            ipc_manager_->AllocRequest(kern_qp, sizeof(labstor_submit_mq_driver_request)));
    kern_submit->Init(MQ_DRIVER_RUNTIME_ID, rq_submit);

    //Complete SERVER -> KERNEL interaction
    qtok = kern_qp->Enqueue(reinterpret_cast<labstor::ipc::request*>(kern_submit));
    kern_complete = reinterpret_cast<labstor_complete_mq_driver_request*>(ipc_manager_->Wait(qtok));

    //Create message for the USER
    rq_complete = reinterpret_cast<labstor_complete_mq_driver_request*>(
            ipc_manager_->AllocRequest(qp, sizeof(labstor_complete_mq_driver_request)));
    rq_complete->header_.op_ = kern_complete->header_.op_;

    //Complete SERVER -> USER interaction
    qp->Complete(
            reinterpret_cast<labstor::ipc::request*>(rq_submit),
            reinterpret_cast<labstor::ipc::request*>(rq_complete));

    //Free used requests
    TRACEPOINT("labstor::MQDriver::Server::IO", "Complete",
               "rq_submit",
               (size_t)rq_submit - (size_t)ipc_manager_->GetRegion(creds->pid),
               "kern_submit",
               (size_t)kern_submit - (size_t)ipc_manager_->GetRegion(KERNEL_PID),
               "kern_complete",
               (size_t)kern_complete - (size_t)ipc_manager_->GetRegion(KERNEL_PID),
               (int)rq_complete->header_.op_);
    ipc_manager_->FreeRequest(qp, reinterpret_cast<labstor::ipc::request*>(rq_submit));
    ipc_manager_->FreeRequest(kern_qp, reinterpret_cast<labstor::ipc::request*>(kern_submit));
}

void labstor::MQDriver::Server::IOComplete(labstor::ipc::queue_pair *qp, labstor_complete_mq_driver_request *rq_complete) {
    AUTO_TRACE("labstor::MQDriver::Server::IOComplete", rq_complete->header_.op_);
}

LABSTOR_MODULE_CONSTRUCT(labstor::MQDriver::Server);