//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "mq_driver.h"
#include "mq_driver_server.h"

bool labstor::MQDriver::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE(request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kGetNumHWQueues: {
            return GetStatistics(qp, reinterpret_cast<mq_driver_request*>(request), creds);
        }
        case Ops::kWrite:
        case Ops::kRead: {
            return IO(qp, reinterpret_cast<mq_driver_request*>(request), creds);
        }
    }
    return true;
}

bool labstor::MQDriver::Server::IO(labstor::queue_pair *qp, mq_driver_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE("dev_id", client_rq->dev_id_, "case", client_rq->GetCode());
    mq_driver_request *kern_rq;
    labstor::queue_pair *kern_qp;

    switch(client_rq->GetCode()) {
        //SUBMIT I/O REQUEST
        case 0: {
            //Get KERNEL QP
            ipc_manager_->GetQueuePairByPid(kern_qp,
                                            LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE |
                                            LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                            KERNEL_PID);

            //Create SERVER -> KERNEL message to submit an I/O request
            TRACEPOINT("Received_req", client_rq->req_id_)
            kern_rq = ipc_manager_->AllocRequest<mq_driver_request>(kern_qp);
            kern_rq->IOKernelStart(MQ_DRIVER_RUNTIME_ID, client_rq);
            kern_qp->Enqueue<mq_driver_request>(kern_rq, client_rq->kern_qtok_);
            client_rq->SetCode(1);
            return false;
        }

        //EXECUTE WHEN THE I/O SUBMISSION FINISHES
        case 1: {
            //Check if the I/O is submitted
            ipc_manager_->GetQueuePair(kern_qp, client_rq->kern_qtok_);
            if(!kern_qp->IsComplete<mq_driver_request>(client_rq->kern_qtok_, kern_rq)) {
                return false;
            }

            //Poll for I/O completion
            if(kern_rq->PollingEnabled()) {
                kern_rq->PollStart();
                kern_qp->Enqueue<mq_driver_request>(kern_rq, client_rq->kern_qtok_);
                client_rq->SetCode(2);
                return false;
            }

            //Wait for interrupt
            else {
                client_rq->kern_rq_ = kern_rq;
                client_rq->SetCode(3);
                return false;
            }
        }

        //I/O POLL COMPLETION CHECK
        case 2: {
            //Check if I/O poll returns
            ipc_manager_->GetQueuePair(kern_qp, client_rq->kern_qtok_);
            if(!kern_qp->IsComplete<mq_driver_request>(client_rq->kern_qtok_, kern_rq)) {
                return false;
            }

            //Re-submit poll request if the I/O is not complete
            if(!kern_rq->IOIsComplete()) {
                kern_qp->Enqueue<mq_driver_request>(kern_rq, client_rq->kern_qtok_);
                return false;
            }

            //Complete the client I/O request
            qp->Complete<mq_driver_request>(client_rq);
            ipc_manager_->FreeRequest<mq_driver_request>(kern_qp, kern_rq);
            return true;
        }

        //INTERRUPT COMPLETION CHECK
        case 3: {
            //Check if interrupt handler completed I/O
            ipc_manager_->GetQueuePair(kern_qp, client_rq->kern_qtok_);
            kern_rq = reinterpret_cast<mq_driver_request*>(client_rq->kern_rq_);
            if(!kern_rq->IOIsComplete()) {
                return false;
            }

            //Complete the client I/O request
            qp->Complete<mq_driver_request>(client_rq);
            ipc_manager_->FreeRequest<mq_driver_request>(kern_qp, kern_rq);
            return true;
        }
    }
    return true;
}

bool labstor::MQDriver::Server::GetStatistics(labstor::queue_pair *qp, mq_driver_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE(client_rq->dev_id_);
    mq_driver_request *kern_rq;
    labstor::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
                                    LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                    KERNEL_PID);

    //Create SERVER -> KERNEL message to submit an I/O request
    kern_rq = ipc_manager_->AllocRequest<mq_driver_request>(kern_qp);
    kern_rq->IOStatsKernelStart(MQ_DRIVER_RUNTIME_ID, client_rq);
    kern_qp->Enqueue<mq_driver_request>(kern_rq, qtok);
    kern_qp->Wait<mq_driver_request>(qtok);

    //Create SERVER -> USER message
    client_rq->Copy(kern_rq);
    qp->Complete<mq_driver_request>(client_rq);

    //Free completed requests
    TRACEPOINT("Complete");
    ipc_manager_->FreeRequest<mq_driver_request>(kern_qp, kern_rq);
    return true;
}

LABSTOR_MODULE_CONSTRUCT(labstor::MQDriver::Server, MQ_DRIVER_MODULE_ID);