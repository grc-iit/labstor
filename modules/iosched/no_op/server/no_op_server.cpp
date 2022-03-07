//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "mq_driver.h"
#include "no_op_server.h"

void labstor::MQDriver::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
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
        case Ops::kIOSubmitComplete: {
            IOSubmitComplete(qp, reinterpret_cast<labstor_mq_driver_poll_request*>(request));
            break;
        }
        case Ops::kIOPollComplete: {
            IOPollComplete(qp, reinterpret_cast<labstor_mq_driver_poll_request*>(request));
            break;
        }
        case Ops::kIOInterruptComplete: {
            IOInterruptComplete(qp, reinterpret_cast<labstor_mq_driver_poll_request*>(request));
            break;
        }
    }
}

void labstor::MQDriver::Server::IOStart(labstor::queue_pair *qp, labstor_mq_driver_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE(client_rq->dev_id_);
    labstor_mq_driver_poll_request *poll_rq;
    labstor_mq_driver_request *kern_rq;
    labstor::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //printf("IOStart\n");

    //Get KERNEL QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
                                    LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE |
                                    LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                    KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE |
                                   LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create SERVER -> KERNEL message to submit an I/O request
    TRACEPOINT("Received_req", client_rq->header_.req_id_)
    kern_rq = ipc_manager_->AllocRequest<labstor_mq_driver_request>(kern_qp);
    kern_rq->IOKernelStart(MQ_DRIVER_RUNTIME_ID, client_rq);
    kern_qp->Enqueue<labstor_mq_driver_request>(kern_rq, qtok);
    //printf("QTOK: %ld %d\n", qtok.qid_, qtok.req_id_);

    //From SERVER, poll the I/O request happening in the KERNEL
    poll_rq = ipc_manager_->AllocRequest<labstor_mq_driver_poll_request>(private_qp);
    poll_rq->IOSubmitInit(qp, client_rq, qtok, kern_rq);
    private_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
}

void labstor::MQDriver::Server::IOSubmitComplete(labstor::queue_pair *private_qp, labstor_mq_driver_poll_request *poll_rq) {
    AUTO_TRACE(poll_rq->op_);
    labstor::queue_pair *client_qp, *kern_qp, *kern_resubmit_qp, *private_resubmit_qp;
    labstor_mq_driver_request *kern_rq;
    labstor_mq_driver_request *client_rq;
    labstor::ipc::qtok_t qtok;

    //Get the queue pairs to re-submit polling requests to
    ipc_manager_->GetQueuePairByPid(kern_resubmit_qp,
                                    LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE |
                                    LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                    KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_resubmit_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Get the kernel request and queue
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->poll_qtok_);
    kern_rq = poll_rq->poll_rq_;

    //Check if the I/O submission is complete
    if(!kern_qp->IsComplete<labstor_mq_driver_request>(poll_rq->poll_qtok_, kern_rq)) {
        private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }

    //Poll for I/O completion
    if(kern_rq->PollingEnabled()) {
        kern_rq->BeginIOPoll();
        poll_rq->IOPollInit();
        kern_resubmit_qp->Enqueue<labstor_mq_driver_request>(kern_rq, poll_rq->poll_qtok_);
        private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }

    //Submit a command to poll for interrupt-based I/O completion
    else {
        poll_rq->IOInterruptInit();
        private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }
}

void labstor::MQDriver::Server::IOPollComplete(labstor::queue_pair *private_qp, labstor_mq_driver_poll_request *poll_rq) {
    AUTO_TRACE(poll_rq->op_);
    labstor::queue_pair *kern_qp, *kern_resubmit_qp, *private_resubmit_qp;
    labstor_mq_driver_request *kern_rq;
    labstor_mq_driver_request *client_rq;
    labstor::ipc::qtok_t qtok;

    //Get the queue pairs to re-submit polling requests to
    ipc_manager_->GetQueuePairByPid(kern_resubmit_qp,
                                    LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE |
                                    LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                    KERNEL_PID);

    ipc_manager_->GetNextQueuePair(private_resubmit_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Get the kernel request and queue
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->poll_qtok_);
    kern_rq = poll_rq->poll_rq_;

    //printf("Checking if kernel poll RQ completed\n");

    //Check if the I/O poll submission is complete
    if(!kern_qp->IsComplete<labstor_mq_driver_request>(poll_rq->poll_qtok_, kern_rq)) {
        private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }

    //printf("Poll completed, but did the I/O[cookie=%d]? %d\n", kern_rq->cookie_, kern_rq->IOIsComplete());

    //Check if the I/O request is complete
    if(!kern_rq->IOIsComplete()) {
        kern_resubmit_qp->Enqueue<labstor_mq_driver_request>(kern_rq, poll_rq->poll_qtok_);
        private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }

    IOComplete(kern_qp, kern_rq, private_qp, poll_rq);
}

void labstor::MQDriver::Server::IOInterruptComplete(labstor::queue_pair *private_qp, labstor_mq_driver_poll_request *poll_rq) {
    AUTO_TRACE(poll_rq->op_);
    labstor::queue_pair *client_qp, *kern_qp, *kern_resubmit_qp, *private_resubmit_qp;
    labstor_mq_driver_request *kern_rq;
    labstor_mq_driver_request *client_rq;
    labstor::ipc::qtok_t qtok;

    //Get the queue pairs to re-submit polling requests to
    ipc_manager_->GetNextQueuePair(private_resubmit_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Get the kernel request and queue
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->poll_qtok_);
    kern_rq = poll_rq->poll_rq_;

    //Check if the I/O request is complete
    if(!kern_rq->IOIsComplete()) {
        private_resubmit_qp->Enqueue<labstor_mq_driver_poll_request>(poll_rq, qtok);
        return;
    }
    IOComplete(kern_qp, kern_rq, private_qp, poll_rq);
}

void labstor::MQDriver::Server::IOComplete(
        labstor::queue_pair *kern_qp,
        labstor_mq_driver_request *kern_rq,
        labstor::queue_pair *private_qp,
        labstor_mq_driver_poll_request *poll_rq) {

    labstor::queue_pair *client_qp;
    labstor_mq_driver_request *client_rq;

    //printf("I/O completed\n");

    //Respond to client
    ipc_manager_->GetQueuePair(client_qp, poll_rq->reply_qtok_);
    client_rq = poll_rq->reply_rq_;
    client_rq->Copy(kern_rq);
    client_qp->Complete<labstor_mq_driver_request>(poll_rq->reply_qtok_, client_rq);

    //Free completed requests
    ipc_manager_->FreeRequest<labstor_mq_driver_request>(kern_qp, kern_rq);
    ipc_manager_->FreeRequest<labstor_mq_driver_poll_request>(private_qp, poll_rq);
}

void labstor::MQDriver::Server::GetStatistics(labstor::queue_pair *qp, labstor_mq_driver_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE(client_rq->dev_id_);
    labstor_mq_driver_request *kern_rq;
    labstor::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
                                    LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                    KERNEL_PID);

    //Create SERVER -> KERNEL message to submit an I/O request
    kern_rq = ipc_manager_->AllocRequest<labstor_mq_driver_request>(kern_qp);
    kern_rq->IOStatsKernelStart(MQ_DRIVER_RUNTIME_ID, client_rq);
    kern_qp->Enqueue<labstor_mq_driver_request>(kern_rq, qtok);
    kern_qp->Wait<labstor_mq_driver_request>(qtok);

    //Create SERVER -> USER message
    client_rq->Copy(kern_rq);
    qp->Complete<labstor_mq_driver_request>(client_rq);

    //Free completed requests
    TRACEPOINT("Complete");
    ipc_manager_->FreeRequest<labstor_mq_driver_request>(kern_qp, kern_rq);
}

LABSTOR_MODULE_CONSTRUCT(labstor::MQDriver::Server, MQ_DRIVER_MODULE_ID);