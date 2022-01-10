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
    Client() : labstor::Module("Registrar") {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    uint32_t RegisterInstance(std::string module_id, std::string key);
    uint32_t GetNamespaceID(std::string str);
};

}

#endif //LABSTOR_REGISTRAR_CLIENT_H
