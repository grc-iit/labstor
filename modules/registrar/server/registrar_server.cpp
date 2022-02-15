//
// Created by lukemartinlogan on 11/26/21.
//

#include <modules/registrar/registrar.h>
#include "registrar_server.h"

void labstor::Registrar::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kRegister : {
            register_request *rq = reinterpret_cast<register_request *>(request);
            labstor::Module *module = module_manager_->GetModuleConstructor(rq->module_id_)();
            module->Initialize(request);
            rq->ConstructModuleEnd(namespace_->AddKey(rq->key_, module));
            printf("Registered module %s: %d\n", rq->key_.key, rq->GetNamespaceID());
            qp->Complete<register_request>(rq);
            break;
        }
        case Ops::kGetNamespaceId : {
            namespace_id_request *rq = reinterpret_cast<namespace_id_request *>(request);
            uint32_t ns_id = namespace_->Get(labstor::ipc::string(rq->key_));
            rq->GetNamespaceIDEnd(ns_id);
            qp->Complete<namespace_id_request>(rq);
            break;
        }
        case Ops::kPushUpgrade: {
            //TODO: finish this
            break;
        }
        case Ops::kTerminate: {
            terminate_request *rq = reinterpret_cast<terminate_request*>(request);
            rq->TerminateEnd();
            qp->Complete<terminate_request>(rq);
            exit(0); //TODO: Make this graceful
            break;
        }
    }
}
LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Server, LABSTOR_REGISTRAR_MODULE_ID)