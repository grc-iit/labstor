//
// Created by lukemartinlogan on 11/26/21.
//

#include <labstor/util/debug.h>
#include <modules/registrar/registrar.h>

#include "dummy.h"
#include "dummy_client.h"

void labstor::test::Dummy::Client::Register() {
    AUTO_TRACE("labstor::test::Dummy::Client::Register")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance("Dummy", "DummyExample");
}

void labstor::test::Dummy::Client::GetValue() {
    AUTO_TRACE("labstor::test::Dummy::Client::GetValue")
    labstor::ipc::queue_pair qp;
    labstor::ipc::qtok_t qtok;
    dummy_submit_request *rq_submit;
    dummy_complete_request *rq_complete;

    ipc_manager_->GetQueuePair(qp, 0);
    rq_submit = reinterpret_cast<dummy_submit_request*>(ipc_manager_->AllocRequest(qp, sizeof(dummy_submit_request)));
    qtok = qp.Enqueue(rq_submit);
    rq_complete = reinterpret_cast<dummy_complete_request*>(ipc_manager_->Wait(qtok));
    printf("COMPLETE: %d\n", rq_complete->num_);
    ipc_manager_->FreeRequest(qtok, rq_complete);
}