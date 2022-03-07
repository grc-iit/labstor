//
// Created by lukemartinlogan on 12/3/21.
//

#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>


#include "ipc_test.h"
#include "ipc_test_client.h"

void labstor::IPCTest::Client::Register() {
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance<labstor::Registrar::register_request>(IPC_TEST_MODULE_ID, IPC_TEST_MODULE_ID);
}

int labstor::IPCTest::Client::GetNamespaceID() {
    auto registrar = labstor::Registrar::Client();
    if(ns_id_ == 0) {
        ns_id_ = registrar.GetNamespaceID(IPC_TEST_MODULE_ID);
        printf("NS ID: %d\n", ns_id_);
    }
    return ns_id_;
}

int labstor::IPCTest::Client::Start(int batch_size) {
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtoks[batch_size];
    labstor_ipc_test_request *client_rq;
    int dev_id;

    ipc_manager_->GetQueuePair(qp, 0);
    //printf("[tid=%d] QID: %lu\n", labstor::ThreadLocal::GetTid(), qp->GetQID().Hash());
    for(int i = 0; i < batch_size; ++i) {
        client_rq = ipc_manager_->AllocRequest<labstor_ipc_test_request>(qp);
        TRACEPOINT("Allocated request")
        client_rq->IPCClientStart(ns_id_, 24);
        qp->Enqueue<labstor_ipc_test_request>(client_rq, qtoks[i]);
        TRACEPOINT("Enqueued request")
    }

    int ret = ipc_manager_->Wait<labstor_ipc_test_request>(qtoks, batch_size);
    TRACEPOINT("Finished wait")
    if (ret != LABSTOR_REQUEST_SUCCESS) {
        printf("IPC test failed: return code %d\n", ret);
        exit(1);
    }
    TRACEPOINT("Finished")
    return dev_id;
}
LABSTOR_MODULE_CONSTRUCT(labstor::IPCTest::Client, IPC_TEST_MODULE_ID)