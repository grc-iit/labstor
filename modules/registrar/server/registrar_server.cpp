//
// Created by lukemartinlogan on 11/26/21.
//

#include <modules/registrar/registrar.h>
#include "registrar_server.h"

bool labstor::Registrar::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kRegister : {
            register_request *rq = reinterpret_cast<register_request *>(request);
            labstor::Module *module = module_manager_->GetModuleConstructor(rq->module_id_)();
            if(module == nullptr) {
                printf("could not find module\n");
                exit(1);
            }
            module->Initialize(request);
            TRACEPOINT("Adding key to namespace", rq->key_.key_, strlen(rq->key_.key_));
            rq->ConstructModuleEnd(namespace_->AddKey(rq->key_, module));
            printf("Registered module %s: %d\n", rq->key_.key_, rq->GetNamespaceID());
            qp->Complete<register_request>(rq);
            return true;
        }
        case Ops::kGetNamespaceId : {
            namespace_id_request *rq = reinterpret_cast<namespace_id_request *>(request);
            TRACEPOINT("Finding key in namespace", rq->key_.key_);
            uint32_t ns_id = namespace_->Get(labstor::ipc::string(rq->key_.key_));
            TRACEPOINT("Key_id", ns_id);
            if(ns_id == -1) {
                rq->GetNamespaceIDEnd(ns_id, LABSTOR_REQUEST_FAILED);
            } else {
                rq->GetNamespaceIDEnd(ns_id, LABSTOR_REQUEST_SUCCESS);
            }
            qp->Complete<namespace_id_request>(rq);
            return true;
        }
        case Ops::kGetModulePath : {
            module_path_request *rq = reinterpret_cast<module_path_request *>(request);
            TRACEPOINT("Finding module in ModuleManager", rq->module_ns_id_);
            labstor::Module *module = namespace_->Get(rq->module_ns_id_);
            std::string path = module_manager_->GetModulePath(module->GetModuleID(), labstor::ModulePathType::kClient);
            TRACEPOINT("PATH", path)
            rq->GetModulePathEnd(path, LABSTOR_REQUEST_SUCCESS);
            qp->Complete<module_path_request>(rq);
            return true;
        }
        case Ops::kPushUpgrade: {
            //TODO: finish this
            return true;
        }
        case Ops::kTerminate: {
            terminate_request *rq = reinterpret_cast<terminate_request*>(request);
            rq->TerminateEnd();
            qp->Complete<terminate_request>(rq);
            exit(0); //TODO: Make this graceful
            return true;
        }
    }
    return true;
}
LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Server, LABSTOR_REGISTRAR_MODULE_ID)