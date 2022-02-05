//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "mq_driver.h"
#include "mq_driver_server.h"

void labstor::MQDriver::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE(request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kGetNumHWQueues: {
            GetStatistics(qp, reinterpret_cast<labstor_mq_driver_request*>(request), creds);
            break;
        }
        case Ops::kWrite:
        case Ops::kRead: {
            IOStart(qp, reinterpret_cast<labstor_mq_driver_request*>(request), creds);
            break;
        }
        case Ops::kIOComplete: {
            IOComplete(qp, reinterpret_cast<labstor_mq_driver_poll_request*>(request));
            break;
        }
    }
}

void labstor::MQDriver::Server::IOStart(labstor::ipc::queue_pair *qp, labstor_mq_driver_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE(client_rq->dev_id_);
    labstor_mq_driver_poll_request *poll_rq;
    labstor_mq_driver_request *kern_rq;
    labstor::ipc::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                               KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_qp,
                               LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create SERVER -> KERNEL message to submit an I/O request
    TRACEPOINT("Received_req", client_rq->header_.req_id_)
    kern_rq = ipc_manager_->AllocRequest<labstor_mq_driver_request>(kern_qp);
    kern_rq->Start(MQ_DRIVER_RUNTIME_ID, client_rq);
    kern_qp->Enqueue<labstor_mq_driver_request>(kern_rq, qtok);

    //From SERVER, poll the I/O request happening in the KERNEL
    poll_rq = ipc_manager_->AllocRequest<labstor_mq_driver_poll_request>(private_qp);
    poll_rq->Init(qp, client_rq, qtok,kern_rq);
    private_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
}

void labstor::MQDriver::Server::IOComplete(labstor::ipc::queue_pair *private_qp, labstor_mq_driver_poll_request *poll_rq) {
    AUTO_TRACE(poll_rq->op_);
    labstor::ipc::queue_pair *client_qp, *kern_qp, *kern_resubmit_qp, *private_resubmit_qp;
    labstor_mq_driver_request *kern_rq;
    labstor_mq_driver_request *client_rq;
    labstor::ipc::qtok_t qtok;

    //Get the queue pairs to re-submit polling requests to
    ipc_manager_->GetQueuePairByPid(kern_resubmit_qp,
                                    LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                    KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_resubmit_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    TRACEPOINT("A");

    //Get the kernel request and queue
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->poll_qtok_);
    kern_rq = poll_rq->poll_rq_;

    TRACEPOINT("B");

    //Case 1: Wait for the I/O to actually become submitted
    if(!kern_rq->IsSubmitted()) {
        //Case 1-1: The I/O has not finished being submitted, re-poll later
        if(!kern_qp->IsComplete<labstor_mq_driver_request>(poll_rq->poll_qtok_, kern_rq)) {
            private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
            return;
        }
        //Case 1-2: The I/O has been submitted
        else {
            TRACEPOINT("C");
            kern_rq->AcknowledgeSubmission();
            if(kern_rq->header_.code_ != LABSTOR_REQUEST_SUCCESS) {
                goto complete_io_request;
            }
            if(kern_rq->PollingEnabled()) {
                kern_rq->BeginIOPoll();
                kern_resubmit_qp->Enqueue<labstor_mq_driver_request>(kern_rq, qtok);
                poll_rq->poll_qtok_ = qtok;
                kern_qp = kern_resubmit_qp;
            }
        }
    }
    //Case 2: Polling-based I/O is used
    if(kern_rq->PollingEnabled()) {
        TRACEPOINT("D");
        if(kern_qp->IsComplete<labstor_mq_driver_request>(poll_rq->poll_qtok_, kern_rq)) {
            if(kern_rq->IOIsComplete() || kern_rq->header_.code_ != LABSTOR_REQUEST_SUCCESS) {
                goto complete_io_request;
            }
            kern_resubmit_qp->Enqueue<labstor_mq_driver_request>(kern_rq, qtok);
            poll_rq->poll_qtok_ = qtok;
        }
        poll_rq->poll_qtok_ = qtok;
        private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }
    //Case 3: Interrupt-based I/O is used
    if(!kern_rq->IOIsComplete()) {
        private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }

    //Create message for the USER
    complete_io_request:
    TRACEPOINT("Completing this request", "qid", poll_rq->reply_qtok_.qid, "req_id", poll_rq->reply_qtok_.req_id, "ret_code", kern_rq->header_.code_)
    ipc_manager_->GetQueuePair(client_qp, poll_rq->reply_qtok_);
    client_rq = poll_rq->reply_rq_;
    client_rq->Copy(kern_rq);
    TRACEPOINT("Current client request", "qid", client_qp->GetQid(), "req_id", client_rq->header_.req_id_, "ret_code", client_rq->header_.code_);

    //Complete SERVER -> USER interaction
    client_qp->Complete<labstor_mq_driver_request>(poll_rq->reply_qtok_, client_rq);

    //Free completed requests
    TRACEPOINT("Complete");
    ipc_manager_->FreeRequest<labstor_mq_driver_request>(kern_qp, kern_rq);
    ipc_manager_->FreeRequest<labstor_mq_driver_poll_request>(private_qp, poll_rq);
}

void labstor::MQDriver::Server::GetStatistics(labstor::ipc::queue_pair *qp, labstor_mq_driver_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE(client_rq->dev_id_);
    labstor_mq_driver_request *kern_rq;
    labstor::ipc::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
                                    LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                    KERNEL_PID);

    //Create SERVER -> KERNEL message to submit an I/O request
    kern_rq = ipc_manager_->AllocRequest<labstor_mq_driver_request>(kern_qp);
    kern_rq->Start(MQ_DRIVER_RUNTIME_ID, client_rq);
    kern_qp->Enqueue<labstor_mq_driver_request>(kern_rq, qtok);
    kern_qp->Wait<labstor_mq_driver_request>(qtok);

    //Create SERVER -> USER message
    client_rq->Copy(kern_rq);
    qp->Complete<labstor_mq_driver_request>(client_rq);

    //Free completed requests
    TRACEPOINT("Complete");
    ipc_manager_->FreeRequest<labstor_mq_driver_request>(kern_qp, kern_rq);
}

LABSTOR_MODULE_CONSTRUCT(labstor::MQDriver::Server);