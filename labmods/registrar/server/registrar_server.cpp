
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <labmods/registrar/registrar.h>
#include "registrar_server.h"

bool labstor::Registrar::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kInit : {
            return Initialize(qp, request, creds);
        }
        case Ops::kGetNamespaceId : {
            namespace_id_request *rq = reinterpret_cast<namespace_id_request *>(request);
            TRACEPOINT("Finding key in namespace", rq->key_.key_);
            uint32_t ns_id = namespace_->GetNamespaceID(labstor::ipc::string(rq->key_.key_));
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
            labstor::Module *module = namespace_->GetModule(rq->module_ns_id_);
            std::string path = module_manager_->GetModulePath(module->GetModuleID(), labstor::ModulePathType::kClient);
            TRACEPOINT("PATH", path)
            rq->GetModulePathEnd(path, LABSTOR_REQUEST_SUCCESS);
            qp->Complete<module_path_request>(rq);
            return true;
        }
        case Ops::kPushUpgrade: {
            module_manager_->PushUpgrade(reinterpret_cast<upgrade_request*>(request));
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

bool labstor::Registrar::Server::Initialize(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    register_request *rq = reinterpret_cast<register_request *>(request);
    labstor::Module *module = module_manager_->GetModuleConstructor(rq->module_id_)();
    if(module == nullptr) {
        printf("could not find module\n");
        exit(1);
    }
    int ns_id = namespace_->AddKey(rq->key_, module);
    TRACEPOINT("Adding key to namespace", rq->key_.key_, strlen(rq->key_.key_));
    rq->ConstructModuleEnd(ns_id);
    printf("Registered module %s: %d\n", rq->key_.key_, rq->GetNamespaceID());
    qp->Complete<register_request>(rq);
    return true;
}


LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Server, LABSTOR_REGISTRAR_MODULE_ID)