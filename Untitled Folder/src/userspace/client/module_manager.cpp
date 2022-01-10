//
// Created by lukemartinlogan on 11/4/21.
//

#include <labstor/userspace/client/client.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/module_manager.h>

void labstor::Client::ModuleManager::UpdateModule(std::string path) {
    //Pause all per-process queues
}

std::string labstor::Client::ModuleManager::GetModulePath(labstor::id module_id) {
    auto ipc_manager = LABSTOR_IPC_MANAGER;
    //Send MODULE_PATH message to trusted server
    return nullptr;
}