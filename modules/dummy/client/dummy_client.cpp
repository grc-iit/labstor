//
// Created by lukemartinlogan on 11/26/21.
//

#include <labstor/userspace/util/debug.h>
#include <modules/registrar/registrar.h>

#include "dummy.h"
#include "dummy_client.h"

void labstor::test::Dummy::Client::Register() {
    AUTO_TRACE("labstor::test::Dummy::Client::Register")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance("Dummy", "DummyExample");
    TRACEPOINT("labstor::test::Dummy::Client::Register::NamespaceID", ns_id_)
}

void labstor::test::Dummy::Client::GetValue() {
    AUTO_TRACE("labstor::test::Dummy::Client::GetValue", ns_id_)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    dummy_submit_request *rq_submit;
    dummy_complete_request *rq_complete;

    ipc_manager_->GetQueuePair(qp, 0);
    TRACEPOINT("labstor::test::Dummy::Client", "Allocating Request")
    rq_submit = ipc_manager_->AllocRequest<dummy_submit_request>(qp);
    rq_submit->Init(ns_id_);
    TRACEPOINT("labstor::test::Dummy::Client", "Enqueuing"
               "SubmitRequestID", rq_submit->req_id_,
               "QP Depth", qp->sq.GetDepth(),
               "QP Max Depth", qp->sq.GetMaxDepth());
    qtok = qp->Enqueue(rq_submit);
    if(LABSTOR_QTOK_INVALID(qtok)) {
        printf("Failed to enqueue\n");
        exit(1);
    }
    TRACEPOINT("labstor::test::Dummy::Client", "Enqueued & Waiting"
                "SubmitRequestID", rq_submit->req_id_,
               "QP Depth", qp->sq.GetDepth(),
               "QP Max Depth", qp->sq.GetMaxDepth());
    rq_complete = ipc_manager_->Wait<dummy_complete_request>(qtok);
    printf("labstor::test::Dummy::Client COMPLETE: %d\n", rq_complete->num_);
    ipc_manager_->FreeRequest(qtok, rq_complete);
}