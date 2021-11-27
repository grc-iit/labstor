//
// Created by lukemartinlogan on 11/4/21.
//

#include <labstor/util/errors.h>
#include <labstor/types/basics.h>
#include <labstor/types/module.h>
#include <labstor/userspace_server/module_manager.h>
#include <labstor/userspace_server/namespace.h>
#include <labstor/userspace_server/ipc_manager.h>

void labstor::Server::ModuleManager::LoadDefaultModules() {
    if(labstor_config_->config_["modules"]) {
        for (const auto &module : labstor_config_->config_["modules"]) {
            labstor::id module_id(module.first.as<std::string>());
            labstor::ModulePath paths;
            if (module.second["client"]) {
                paths.client = module.second["client"].as<std::string>();
            }
            if (module.second["server"]) {
                paths.server = module.second["server"].as<std::string>();
            }
            AddModulePaths(module_id, paths);
            UpdateModule(paths.server);
        }
    }
}

void labstor::Server::ModuleManager::UpdateModule(std::string path) {
    labstor::id module_id;
    labstor::ModuleHandle module_info;
    labstor::Module *old_instance, *new_instance;
    uint32_t runtime_id;
    LABSTOR_NAMESPACE_T namespace_ = LABSTOR_NAMESPACE;
    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;

    //Pause all queues & wait until there are no busy queues
    ipc_manager_->PauseQueues();
    ipc_manager_->WaitForPause();

    //Process update
    LABSTOR_ERROR_HANDLE_TRY {
        module_info = OpenModule(path, module_id);
        std::queue<labstor::Module*> &modules = namespace_->AllModuleInstances(module_id);
        for(int i = 0; i < modules.size(); ++i) {
            labstor::Module *old_instance = modules.front();
            modules.pop();
            new_instance = module_info.constructor_();
            new_instance->StateUpdate(old_instance);
            modules.push(new_instance);
            delete old_instance;
        }
        SetModuleConstructor(module_id, module_info);
    } LABSTOR_ERROR_HANDLE_CATCH {
        ipc_manager_->ResumeQueues();
        throw err;
    }

    //Resume all queues
    ipc_manager_->ResumeQueues();
}

void labstor::Server::ModuleManager::AddModulePaths(labstor::id module_id, labstor::ModulePath paths) {
    paths_[module_id] = paths;
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
    return nullptr;
}