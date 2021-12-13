//
// Created by lukemartinlogan on 11/26/21.
//

#include "registrar.h"
#include "registrar_client.h"

uint32_t labstor::Registrar::Client::RegisterInstance(std::string module_id, std::string key) {
    AUTO_TRACE("labstor::Registrar::Client::RegisterInstance", "module", module_id, "key", key)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::register_submit_request *rq_submit;
    labstor::Registrar::register_complete_request *rq_complete;

    ipc_manager_->GetQueuePair(qp, 0);
    TRACEPOINT("labstor::Registrar::Client::RegisterInstance", "Allocating request")
    rq_submit = ipc_manager_->AllocRequest<register_submit_request>(qp);
    rq_submit->Init(module_id, key);
    TRACEPOINT("labstor::Registrar::Client::RegisterInstance", "Enqueue request")
    qp->Enqueue(rq_submit, qtok);
    TRACEPOINT("labstor::Registrar::Client::RegisterInstance", "Wait for request")
    rq_complete =ipc_manager_->Wait<register_complete_request>(qtok);
    TRACEPOINT("labstor::Registrar::Client::RegisterInstance", "Complete request")
    uint32_t ns_id = rq_complete->ns_id_;
    TRACEPOINT("labstor::Registrar::Client::RegisterInstance", ns_id)
    ipc_manager_->FreeRequest(qtok, rq_complete);
    return ns_id;
}

uint32_t labstor::Registrar::Client::GetNamespaceID(std::string key) {
    AUTO_TRACE("labstor::Registrar::Client::GetNamespaceID", "key", key)
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::namespace_id_submit_request *rq_submit;
    labstor::Registrar::namespace_id_complete_request *rq_complete;
    uint32_t ns_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq_submit = ipc_manager_->AllocRequest<namespace_id_submit_request>(qp);
    rq_submit->Init(key);
    qp->Enqueue(rq_submit, qtok);
    rq_complete = ipc_manager_->Wait<namespace_id_complete_request>(qtok);
    ns_id = rq_complete->ns_id_;
    //ipc_manager_->FreeRequest(qtok, rq_complete);
    return ns_id;
}

LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Client)