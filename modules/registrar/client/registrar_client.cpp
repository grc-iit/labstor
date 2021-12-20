//
// Created by lukemartinlogan on 11/26/21.
//


#include "registrar.h"
#include "registrar_client.h"

uint32_t labstor::Registrar::Client::RegisterInstance(std::string module_id, std::string key) {
    AUTO_TRACE("module", module_id, "key", key)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::register_request *rq;

    ipc_manager_->GetQueuePair(qp, 0);
    TRACEPOINT("Allocating request")
    rq = ipc_manager_->AllocRequest<register_request>(qp);
    rq->Start(module_id, key);
    TRACEPOINT("Enqueue request")
    qp->Enqueue(rq, qtok);
    TRACEPOINT("Wait for request", qtok.qid, qp->sq.queue_.header_->enqueued_);
    rq = ipc_manager_->Wait<register_request>(qtok);
    TRACEPOINT("Complete request")
    uint32_t ns_id = rq->ns_id_;
    TRACEPOINT(ns_id)
    ipc_manager_->FreeRequest(qtok, rq);
    return ns_id;
}

uint32_t labstor::Registrar::Client::GetNamespaceID(std::string key) {
    AUTO_TRACE("key", key)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::namespace_id_request *rq;
    uint32_t ns_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<namespace_id_request>(qp);
    rq->Start(key);
    qp->Enqueue(rq, qtok);
    rq = ipc_manager_->Wait<namespace_id_request>(qtok);
    ns_id = rq->ns_id_;
    ipc_manager_->FreeRequest(qtok, rq);
    return ns_id;
}

LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Client)