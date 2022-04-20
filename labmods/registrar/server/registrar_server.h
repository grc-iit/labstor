//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_REGISTRAR_SERVER_H
#define LABSTOR_REGISTRAR_SERVER_H

#include "registrar.h"
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>

namespace labstor::Registrar {

class Server : public labstor::Module {
private:
    LABSTOR_MODULE_MANAGER_T module_manager_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
public:
    Server() : labstor::Module(LABSTOR_REGISTRAR_MODULE_ID) {
        module_manager_ = LABSTOR_MODULE_MANAGER;
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    bool Initialize(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
    bool ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
};

}

#endif //LABSTOR_REGISTRAR_SERVER_H
