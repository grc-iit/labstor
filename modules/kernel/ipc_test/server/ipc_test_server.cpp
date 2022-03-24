//
// Created by lukemartinlogan on 11/26/21.
//

#include "ipc_test_server.h"
#include <labstor/constants/constants.h>

bool labstor::IPCTest::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kStartIPCTest: {
            return IPC(qp, reinterpret_cast<ipc_test_request *>(request));
        }
    }
    return true;
}

bool labstor::IPCTest::Server::IPC(labstor::queue_pair *qp, ipc_test_request *client_rq) {
    AUTO_TRACE("labstor::IPCTest::Server::Start");
    ipc_test_request *kern_rq;
    labstor::queue_pair *kern_qp;

    //Get KERNEL & PRIVATE QP
    switch(client_rq->GetCode()) {
        case 0: {
            ipc_manager_->GetQueuePairByPid(kern_qp,
                                            LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                            KERNEL_PID);

            //Create SERVER -> KERNEL message
            kern_rq = ipc_manager_->AllocRequest<ipc_test_request>(kern_qp);
            kern_rq->IPCKernelStart(IPC_TEST_MODULE_RUNTIME_ID);
            kern_qp->Enqueue<ipc_test_request>(kern_rq, client_rq->kern_qtok_);
            client_rq->SetCode(1);
            return false;
        }
        case 1: {
            //Check if the QTOK has been completed
            ipc_manager_->GetQueuePair(kern_qp, client_rq->kern_qtok_);
            if(!kern_qp->IsComplete<ipc_test_request>(client_rq->kern_qtok_, kern_rq)) {
                return false;
            }

            //Create message for the USER
            client_rq->Copy(kern_rq);

            //Complete SERVER -> USER interaction
            TRACEPOINT("client_return_code", client_rq->GetCode());
            qp->Complete<ipc_test_request>(client_rq);

            //Free completed requests
            ipc_manager_->FreeRequest<ipc_test_request>(kern_qp, kern_rq);
            return true;
        }
    }

    return true;
}

LABSTOR_MODULE_CONSTRUCT(labstor::IPCTest::Server, IPC_TEST_MODULE_ID)