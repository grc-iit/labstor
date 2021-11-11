//
// Created by lukemartinlogan on 11/4/21.
//

#include <labstor/types/basics.h>
#include <labstor/types/module.h>

void labstor::Server::ModuleManager::UpdateModule(std::string path) {
    labstor::Module *new_pkg, *old_pkg;
    uint32_t runtime_id;
    //Pause all queues & wait until there are no busy queues
    new_pkg = OpenModule();
    old_pkg = GetModule(new_pkg->GetModuleID());
    if(old_pkg) {
        runtime_id = old_pkg->GetRuntimeID();
        new_pkg->SetRuntimeID(runtime_id);
        pkg_pool_[runtime_id] = new_pkg;
    } else {
        runtime_id = cur_runtime_id++;
    }
}

void labstor::Server::ModuleManager::AddModulePaths(labstor::id module_id, labstor::ModulePath paths) {
    mutex_.lock();
    paths_[module_id] = paths;
    mutex_.unlock();
}

std::string labstor::Server::ModuleManager::GetModulePath(labstor::id module_id, ModulePathType type) {
    switch(type) {
        case ModulePathType::CLIENT: {
            return paths_[module_id].client;
        }
        case ModulePathType::SERVER: {
            return paths_[module_id].server;
        }
    }
}