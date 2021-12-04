//
// Created by lukemartinlogan on 11/4/21.
//

#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/util/debug.h>
#include <labstor/userspace/util/path_parser.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/server/namespace.h>
#include <labstor/userspace/server/ipc_manager.h>

void labstor::Server::ModuleManager::LoadDefaultModules() {
    AUTO_TRACE("labstor::Server::ModuleManager::LoadDefaultModules")
    if(labstor_config_->config_["modules"]) {
        for (const auto &module : labstor_config_->config_["modules"]) {
            labstor::id module_id(module.first.as<std::string>());
            labstor::ModulePath paths;
            if (module.second["client"]) {
                paths.client = scs::path_parser(module.second["client"].as<std::string>());
            }
            if (module.second["server"]) {
                paths.server = scs::path_parser(module.second["server"].as<std::string>());
            }
            AddModulePaths(module_id, paths);
            UpdateModule(paths.server);
        }
    }
}

void labstor::Server::ModuleManager::UpdateModule(std::string path) {
    AUTO_TRACE("labstor::Server::ModuleManager::UpdateModule", path)
    labstor::id module_id;
    labstor::ModuleHandle module_info;
    labstor::Module *old_instance, *new_instance;
    labstor_runtime_id_t runtime_id;
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
    AUTO_TRACE("labstor::Server::ModuleManager::AddModulePaths")
    paths_[module_id] = paths;
}

std::string labstor::Server::ModuleManager::GetModulePath(labstor::id module_id, ModulePathType type) {
    AUTO_TRACE("labstor::Server::ModuleManager::GetModulePath")
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