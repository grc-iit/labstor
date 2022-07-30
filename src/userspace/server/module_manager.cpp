
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

#include <filesystem>
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/constants/debug.h>
#include <labstor/userspace/util/path_parser.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/server/namespace.h>
#include <labstor/userspace/server/ipc_manager.h>

bool labstor::Server::ModuleManager::LoadRepos() {
    AUTO_TRACE("")
    if(labstor_config_->config_["repos"]) {
        for(auto repo : labstor_config_->config_["repos"]) {
            std::string repo_path = scs::path_parser(repo.as<std::string>());
            printf("Processing repo: %s\n", repo_path.c_str());
            std::string labmod_uuid_path = repo_path + "/include/labmods";
            labstor::ModulePath paths;
            if(!std::filesystem::exists(labmod_uuid_path)) {
                printf("%s doesn't exist in repo yaml\n", labmod_uuid_path.c_str());
                return false;
            }
            for(auto &dir_entry : std::filesystem::directory_iterator(labmod_uuid_path)) {
                std::string labmod_uuid_str = dir_entry.path().filename();
                labstor::id labmod_uuid(labmod_uuid_str);
                if(HasModule(labmod_uuid)) {
                    continue;
                }
                std::string client_lib = repo_path + "/lib/" + "lib" + labmod_uuid_str + "_client.so";
                std::string server_lib = repo_path + "/lib/" + "lib" + labmod_uuid_str + "_server.so";
                if(std::filesystem::exists(client_lib)) {
                    paths.client = client_lib;
                }
                if(std::filesystem::exists(server_lib)) {
                    paths.server = server_lib;
                    labstor::ModuleHandle module_info = OpenModule(paths.server, labmod_uuid);
                    SetModuleConstructor(labmod_uuid, module_info);
                }
                AddModulePaths(labmod_uuid, paths);
                printf("Added module %s\n", labmod_uuid_str.c_str());
                printf("  Client path: %s\n", paths.client.c_str());
                printf("  Server path: %s\n", paths.server.c_str());
            }
        }
    }
    return true;
}

void labstor::Server::ModuleManager::CentralizedUpdateModule(YAML::Node config) {
    AUTO_TRACE("")
    std::string path = config["code_upgrade"]["centralized"].as<std::string>();
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

void labstor::Server::ModuleManager::DecentralizedUpdateModule(YAML::Node config) {
}

void labstor::Server::ModuleManager::AddModulePaths(labstor::id module_id, labstor::ModulePath paths) {
    AUTO_TRACE("")
    paths_[module_id] = paths;
}

std::string labstor::Server::ModuleManager::GetModulePath(labstor::id module_id, ModulePathType type) {
    AUTO_TRACE("")
    switch(type) {
        case ModulePathType::kClient: {
            return paths_[module_id].client;
        }
        case ModulePathType::kServer: {
            return paths_[module_id].server;
        }
    }
    return nullptr;
}