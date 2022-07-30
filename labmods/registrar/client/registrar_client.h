
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

#ifndef LABSTOR_REGISTRAR_CLIENT_H
#define LABSTOR_REGISTRAR_CLIENT_H

#include <labstor/userspace/client/client.h>
#include "labmods/registrar/registrar.h"
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/types/module.h>

namespace labstor::Registrar {

class Client : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Client() : labstor::Module(LABSTOR_REGISTRAR_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Register(YAML::Node config) override {}
    void Initialize(int ns_id) override {}
    template<typename register_request_t, typename ...Args>
    uint32_t InitializeInstance(Args ...args) {
        labstor::queue_pair *qp;
        labstor::ipc::qtok_t qtok;
        register_request_t *rq;

        ipc_manager_->GetQueuePair(qp, 0);
        rq = ipc_manager_->AllocRequest<register_request_t>(qp);
        rq->ConstructModuleStart(args...);
        qp->Enqueue(rq, qtok);
        rq = ipc_manager_->Wait<register_request_t>(qtok);
        uint32_t ns_id = rq->ns_id_;
        ipc_manager_->FreeRequest(qtok, rq);
        return ns_id;
    }
    uint32_t RegisterInstance(std::string module_id, std::string key) {
        return InitializeInstance<register_request>(std::move(module_id), std::move(key));
    }
    uint32_t GetNamespaceID(std::string str);
    std::string GetModulePath(int ns_id);
    int MountLabStack(std::string key, std::string yaml_path);
    int UnmountLabStack(std::string key, std::string yaml_path);
    int PushUpgrade(std::string yaml_path);
    void TerminateServer();
};

}

#endif //LABSTOR_REGISTRAR_CLIENT_H