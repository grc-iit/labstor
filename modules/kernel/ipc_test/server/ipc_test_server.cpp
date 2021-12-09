//
// Created by lukemartinlogan on 11/26/21.
//

#include "ipc_test_server.h"
#include <labstor/constants/constants.h>

void labstor::IPCTest::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    //AUTO_TRACE("labstor::IPCTest::ProcessRequest", request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kStartIPCTest: {
            Start(qp, reinterpret_cast<labstor_submit_ipc_test_request *>(request));
            break;
        }
        case Ops::kCompleteIPCTest: {
            End(qp, reinterpret_cast<labstor_poll_ipc_test_request*>(request));
            break;
        }
    }
}

void labstor::IPCTest::Server::Start(labstor::ipc::queue_pair *qp, labstor_submit_ipc_test_request *rq_submit) {
    AUTO_TRACE("labstor::IPCTest::Server::Start");
    labstor_poll_ipc_test_request *poll_rq;
    labstor_submit_ipc_test_request *kern_submit;
    labstor::ipc::queue_pair *kern_qp, *private_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL & PRIVATE QP
    ipc_manager_->GetQueuePairByPid(kern_qp,
       LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
       KERNEL_PID);
    ipc_manager_->GetNextQueuePair(private_qp,
                                   LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create SERVER -> KERNEL message
    kern_submit = ipc_manager_->AllocRequest<labstor_submit_ipc_test_request>(kern_qp);
    kern_submit->Init(IPC_TEST_MODULE_RUNTIME_ID);
    TRACEPOINT("labstor::IPCTest::Server::Start", "start_req_id", rq_submit->header_.GetRequestID(), "start_qp_id", qp->GetQid());
    qtok = kern_qp->Enqueue<labstor_submit_ipc_test_request>(kern_submit);

    //Poll SERVER -> KERNEL interaction
    TRACEPOINT("labstor::IPCTest::Server::Start", "Allocating Poll RQ")
    poll_rq = ipc_manager_->AllocRequest<labstor_poll_ipc_test_request>(private_qp);
    poll_rq->Init(qp, rq_submit, qtok);
    private_qp->Enqueue<labstor_poll_ipc_test_request>(poll_rq);

    //Release requests
    ipc_manager_->FreeRequest<labstor_submit_ipc_test_request>(qp, rq_submit);
}

void labstor::IPCTest::Server::End(labstor::ipc::queue_pair *private_qp, labstor_poll_ipc_test_request *poll_rq) {
    //AUTO_TRACE("labstor::IPCTest::Server::End");
    labstor::ipc::queue_pair *qp, *kern_qp;
    labstor_complete_ipc_test_request *kern_complete;
    labstor_complete_ipc_test_request *rq_complete;

    //Check if the QTOK has been completed
    //TRACEPOINT("labstor::IPCTest::Server::End", "Check if I/O has completed")
    ipc_manager_->GetQueuePair(kern_qp, poll_rq->kqtok_);
    if(!kern_qp->IsComplete<labstor_complete_ipc_test_request>(poll_rq->kqtok_, kern_complete)) {
        private_qp->Enqueue<labstor_poll_ipc_test_request>(poll_rq);
        return;
    }

    //Create message for the USER
    TRACEPOINT("labstor::IPCTest::Server::End")
    ipc_manager_->GetQueuePair(qp, poll_rq->uqtok_);
    rq_complete = ipc_manager_->AllocRequest<labstor_complete_ipc_test_request>(qp);
    rq_complete->Copy(kern_complete);

    //Complete SERVER -> USER interaction
    TRACEPOINT("labstor::IPCTest::Server::End", "end_rq_id", poll_rq->uqtok_.req_id, "end_qid", poll_rq->uqtok_.qid)
    qp->Complete<labstor_complete_ipc_test_request>(poll_rq->uqtok_, rq_complete);

    //Free completed requests
    TRACEPOINT("labstor::IPCTest::Server::End", "Free",
               "return_code",
               rq_complete->GetReturnCode());
    ipc_manager_->FreeRequest<labstor_complete_ipc_test_request>(kern_qp, kern_complete);
    ipc_manager_->FreeRequest<labstor_poll_ipc_test_request>(private_qp, poll_rq);
}

LABSTOR_MODULE_CONSTRUCT(labstor::IPCTest::Server)