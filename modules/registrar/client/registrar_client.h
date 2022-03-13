//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_REGISTRAR_CLIENT_H
#define LABSTOR_REGISTRAR_CLIENT_H

#include "modules/registrar/registrar.h"
#include <labstor/userspace/types/module.h>
#include <labstor/types/data_structures/shmem_request.h>

#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>

namespace labstor::Registrar {

class Client : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Client() : labstor::Module(LABSTOR_REGISTRAR_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Initialize(labstor::ipc::request *rq) {}

    template<typename register_request_t, typename ...Args>
    uint32_t RegisterInstance(std::string module_id, std::string key, Args ...args) {
        labstor::queue_pair *qp;
        labstor::ipc::qtok_t qtok;
        register_request_t *rq;

        ipc_manager_->GetQueuePair(qp, 0);
        rq = ipc_manager_->AllocRequest<register_request_t>(qp);
        rq->ConstructModuleStart(module_id, key, args...);
        qp->Enqueue(rq, qtok);
        rq = ipc_manager_->Wait<register_request_t>(qtok);
        uint32_t ns_id = rq->ns_id_;
        ipc_manager_->FreeRequest(qtok, rq);
        return ns_id;
    }
    uint32_t GetNamespaceID(std::string str);
    std::string GetModulePath(int ns_id);
    void PushUpgrade(std::string key);
    void TerminateServer();
};

}

#endif //LABSTOR_REGISTRAR_CLIENT_H
