//
// Created by lukemartinlogan on 11/4/21.
//

#include <labstor/types/basics.h>
#include <labstor/types/module.h>

void labstor::Client::ModuleManager::UpdateModule(std::string path) {
    //Pause all per-process queues
}

std::string labstor::Client::ModuleManager::GetModulePath(labstor::id module_id) {
    auto ipc_manager = scs::Singleton<labstor::Client::IPCManager>::GetInstance();
    ipc_manager.GetQueue();
    //Send MODULE_PATH message to trusted server
}