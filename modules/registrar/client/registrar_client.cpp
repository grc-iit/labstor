//
// Created by lukemartinlogan on 11/26/21.
//


#include "registrar.h"
#include "registrar_client.h"

uint32_t labstor::Registrar::Client::GetNamespaceID(std::string key) {
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::namespace_id_request *rq;
    uint32_t ns_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<namespace_id_request>(qp);
    rq->GetNamespaceIDStart(key);
    qp->Enqueue(rq, qtok);
    rq = ipc_manager_->Wait<namespace_id_request>(qtok);
    ns_id = rq->GetNamespaceID();
    ipc_manager_->FreeRequest(qtok, rq);
    return ns_id;
}

void labstor::Registrar::Client::PushUpgrade(std::string key) {
}

void labstor::Registrar::Client::TerminateServer() {
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::terminate_request *rq;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<terminate_request>(qp);
    rq->TerminateStart();
    qp->Enqueue(rq, qtok);
    rq = ipc_manager_->Wait<terminate_request>(qtok);
    ipc_manager_->FreeRequest(qtok, rq);
}

LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Client, LABSTOR_REGISTRAR_MODULE_ID)