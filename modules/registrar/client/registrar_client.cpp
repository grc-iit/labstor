//
// Created by lukemartinlogan on 11/26/21.
//

#include "registrar.h"
#include "registrar_client.h"

uint32_t labstor::Registrar::Client::RegisterInstance(std::string module_id, std::string key) {
    labstor::ipc::queue_pair qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::register_submit_request *rq_submit;
    labstor::Registrar::register_complete_request *rq_complete;

    ipc_manager_->GetQueuePair(qp, 0);
    rq_submit = reinterpret_cast<register_submit_request*>(ipc_manager_->AllocRequest(qp, sizeof(labstor::Registrar::register_submit_request)));
    rq_submit->Init(module_id, key);
    qtok = qp.Enqueue(rq_submit);
    rq_complete = reinterpret_cast<register_complete_request*>(ipc_manager_->Wait(qtok));
    uint32_t ns_id = rq_complete->ns_id_;
    ipc_manager_->FreeRequest(qtok, rq_complete);
    return ns_id;
}

uint32_t labstor::Registrar::Client::GetNamespaceID(std::string key) {
    labstor::ipc::queue_pair qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::namespace_id_submit_request *rq_submit;
    labstor::Registrar::namespace_id_complete_request *rq_complete;
    uint32_t ns_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq_submit = reinterpret_cast<namespace_id_submit_request*>(ipc_manager_->AllocRequest(qp, sizeof(labstor::Registrar::register_submit_request)));
    rq_submit->Init(key);
    qtok = qp.Enqueue(rq_submit);
    rq_complete = reinterpret_cast<namespace_id_complete_request*>(ipc_manager_->Wait(qtok));
    ns_id = rq_complete->ns_id_;
    ipc_manager_->FreeRequest(qtok, rq_complete);
    return ns_id;
}