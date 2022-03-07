//
// Created by lukemartinlogan on 11/26/21.
//

#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "dummy.h"
#include "dummy_client.h"

void labstor::test::Dummy::Client::Register() {
    AUTO_TRACE("")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance<labstor::Registrar::register_request>(LABSTOR_DUMMY_MODULE_ID, LABSTOR_DUMMY_MODULE_ID);
    TRACEPOINT(ns_id_)
}

int labstor::test::Dummy::Client::GetNamespaceID() {
    auto registrar = labstor::Registrar::Client();
    if(ns_id_ == 0) {
        ns_id_ = registrar.GetNamespaceID(LABSTOR_DUMMY_MODULE_ID);
    }
    return ns_id_;
}

void labstor::test::Dummy::Client::GetValue() {
    AUTO_TRACE(ns_id_)
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    dummy_request *rq;
    labstor::HighResCpuTimer t;
    std::vector<labstor::ipc::qtok_t> qtoks;

    ipc_manager_->GetQueuePair(qp, 0);
    for(int i = 0; i < 100; ++i) {
        rq = ipc_manager_->AllocRequest<dummy_request>(qp);
        rq->Start(ns_id_);
        TRACEPOINT("START", t.GetUsFromEpoch());
        qp->Enqueue(rq, qtok);
        qtoks.emplace_back(qtok);
    }

    for(int i = 0; i < 100; ++i) {
        rq = ipc_manager_->Wait<dummy_request>(qtoks[i]);
        TRACEPOINT("END", t.GetUsFromEpoch());
        ipc_manager_->FreeRequest(qtok, rq);
    }
}

LABSTOR_MODULE_CONSTRUCT(labstor::test::Dummy::Client, LABSTOR_DUMMY_MODULE_ID)