//
// Created by lukemartinlogan on 12/3/21.
//
#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "ipc_test.h"
#include "ipc_test_client.h"

void labstor::IPCTest::Client::Register() {
    AUTO_TRACE("labstor::IPCTest::Register::Client")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance(IPC_TEST_MODULE_ID, IPC_TEST_MODULE_ID);
    TRACEPOINT("labstor::IPCTest::Client::Register::NamespaceID", ns_id_)
}

int labstor::IPCTest::Client::Start() {
    AUTO_TRACE("labstor::IPCTest::Client::AddBdev", ns_id_)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor_submit_ipc_test_request *rq_submit;
    labstor_complete_ipc_test_request *rq_complete;
    int dev_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq_submit = ipc_manager_->AllocRequest<labstor_submit_ipc_test_request>(qp);
    rq_submit->Init(ns_id_, 24);

    TRACEPOINT("labstor::IPCTest::Client::Enqueue", rq_submit->header_.ns_id_);
    qp->Enqueue<labstor_submit_ipc_test_request>(rq_submit, qtok);
    TRACEPOINT("labstor::IPCTest::Client::Enqueue", "req_id", qtok.req_id, "qid", qtok.qid, "qdepth", qp->GetDepth());
    rq_complete = ipc_manager_->Wait<labstor_complete_ipc_test_request>(qtok);
    int ret = rq_complete->GetReturnCode();
    TRACEPOINT("labstor::IPCTest::Client::Start", "Complete",
               "return_code", ret)
    ipc_manager_->FreeRequest<labstor_complete_ipc_test_request>(qtok, rq_complete);

    if(ret != IPC_TEST_SUCCESS) {
        printf("IPC test failed: return code %d\n", ret);
        exit(1);
    }
    return dev_id;
}
LABSTOR_MODULE_CONSTRUCT(labstor::IPCTest::Client)