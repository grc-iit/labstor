//
// Created by lukemartinlogan on 11/26/21.
//

#include <modules/registrar/registrar.h>
#include "registrar_server.h"

void labstor::Registrar::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("", request->ns_id_, request->op_, request->req_id_, creds->pid);
    switch(static_cast<Ops>(request->op_)) {
        case Ops::kRegister : {
            register_request *rq = reinterpret_cast<register_request*>(request);
            labstor::Module *module = module_manager_->GetModuleConstructor(rq->module_id_)();
            rq->Complete(namespace_->AddKey(rq->key_, module));
            qp->Complete<register_request>(rq);
            break;
        }
        case Ops::kGetNamespaceId : {
            namespace_id_request *rq = reinterpret_cast<namespace_id_request*>(request);
            uint32_t ns_id = namespace_->Get(labstor::ipc::string(rq->key_));
            rq->Complete(ns_id);
            qp->Complete<namespace_id_request>(rq);
            break;
        }
    }
}
LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Server)