//
// Created by lukemartinlogan on 11/26/21.
//

#include <modules/registrar/registrar.h>
#include "registrar_server.h"

void labstor::Registrar::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("labstor::Registrar::Server::ProcessRequest", request->ns_id_, request->op_, request->req_id_, creds->pid);

    switch(static_cast<Ops>(request->op_)) {
        case Ops::kRegister : {
            register_submit_request *register_rq = reinterpret_cast<register_submit_request*>(request);
            register_complete_request *register_complete =
                    ipc_manager_->AllocRequest<register_complete_request>(qp, sizeof(register_complete_request));
            labstor::Module *module = module_manager_->GetModuleConstructor(register_rq->module_id_)();
            register_complete->Init(namespace_->AddKey(register_rq->key_, module));
            TRACEPOINT("labstor::Registrar::Server::ProcessRequest", "NamespaceID", register_complete->ns_id_);
            qp->Complete<register_submit_request,register_complete_request>(register_rq, register_complete);
            ipc_manager_->FreeRequest(qp, register_rq);
            break;
        }
        case Ops::kGetNamespaceId : {
            namespace_id_submit_request *ns_get = reinterpret_cast<namespace_id_submit_request*>(request);
            namespace_id_complete_request *ns_get_complete;
            uint32_t ns_id = namespace_->Get(labstor::ipc::string(ns_get->key_));
            ns_get_complete = ipc_manager_->AllocRequest<namespace_id_complete_request>(qp, sizeof(namespace_id_complete_request));
            ns_get_complete->Init(ns_id);
            qp->Complete(ns_get, ns_get_complete);
            ipc_manager_->FreeRequest(qp, ns_get);
            break;
        }
    }

}
LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Server)