//
// Created by lukemartinlogan on 11/26/21.
//

#include "ipc_test_server.h"
#include <labstor/constants/constants.h>

void labstor::IPCTest::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kStartIPCTest: {
            Start(qp, reinterpret_cast<labstor_ipc_test_request *>(request));
            break;
        }
        case Ops::kCompleteIPCTest: {
            End(qp, reinterpret_cast<labstor_poll_ipc_test_request*>(request));
            break;
        }
    }
}

void labstor::IPCTest::Server::Start(labstor::queue_pair *qp, labstor_ipc_test_request *client_rq) {
    AUTO_TRACE("labstor::IPCTest::Server::Start");
    labstor_poll_ipc_test_request *poll_rq;
    labstor_ipc_test_request *kern_rq;
    labstor::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL & PRIVATE QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
       LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
       KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create SERVER -> KERNEL message
    kern_rq = ipc_manager_->AllocRequest<labstor_ipc_test_request>(kern_qp);
    kern_rq->IPCKernelStart(IPC_TEST_MODULE_RUNTIME_ID);
    kern_qp->Enqueue<labstor_ipc_test_request>(kern_rq, qtok);

    //Poll SERVER -> KERNEL interaction.
    TRACEPOINT("Allocating Poll RQ")
    poll_rq = ipc_manager_->AllocRequest<labstor_poll_ipc_test_request>(private_qp);
    poll_rq->Init(qp, client_rq, qtok);
    private_qp->Enqueue<labstor_poll_ipc_test_request>(poll_rq, qtok);
}

void labstor::IPCTest::Server::End(labstor::queue_pair *private_qp, labstor_poll_ipc_test_request *poll_rq) {
    labstor::queue_pair *qp, *kern_qp, *resubmit_qp;
    labstor_ipc_test_request *kern_rq, *client_rq;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL & PRIVATE QP
    ipc_manager_->GetNextQueuePair(resubmit_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Check if the QTOK has been completed
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->poll_qtok_);
    if(!kern_qp->IsComplete<labstor_ipc_test_request>(poll_rq->poll_qtok_, kern_rq)) {
        resubmit_qp->Enqueue<labstor_poll_ipc_test_request>(poll_rq, qtok);
        return;
    }

    //Create message for the USER
    TRACEPOINT("kernel_return_code", kern_rq->GetReturnCode());
    ipc_manager_->GetQueuePair(qp, poll_rq->reply_qtok_);
    client_rq = poll_rq->reply_rq_;
    client_rq->Copy(kern_rq);

    //Complete SERVER -> USER interaction
    TRACEPOINT("client_return_code",
               client_rq->GetReturnCode());
    qp->Complete<labstor_ipc_test_request>(poll_rq->reply_qtok_, client_rq);

    //Free completed requests
    TRACEPOINT("client_return_code",
               client_rq->GetReturnCode());
    ipc_manager_->FreeRequest<labstor_ipc_test_request>(kern_qp, kern_rq);
    ipc_manager_->FreeRequest<labstor_poll_ipc_test_request>(private_qp, poll_rq);
    TRACEPOINT("Completion")
}

LABSTOR_MODULE_CONSTRUCT(labstor::IPCTest::Server, IPC_TEST_MODULE_ID)