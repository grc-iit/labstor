//
// Created by lukemartinlogan on 11/26/21.
//

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
    void PushUpgrade(std::string key);
    void TerminateServer();
};

}

#endif //LABSTOR_REGISTRAR_CLIENT_H
