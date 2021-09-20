//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_SERVER_H
#define LABSTOR_SERVER_H

#include <labstor/types/ipc_pool.h>
#include <labstor/types/module.h>
#include <labstor/ipc/unordered_map.h>
#include <labstor/userspace_server/ipc_manager.h>
#include <labstor/userspace_server/work_orchestrator.h>

#include <pthread.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace labstor {

struct LabStorServerContext {
    int pid_;
    YAML::Node config_;
    labstor::ipc::unordered_map<std::string, labstor::module> namespace_;
    std::unordered_map <pid_t, std::shared_ptr<labstor::PerProcessIPC>> ipc_pool_;
    labstor::ModuleManager module_manager_;
    labstor::IPCManager ipc_manager_;
    labstor::WorkOrchestrator work_orchestrator_;
    int accept_fd_;

    void LoadConfig(char *path) {
        config_ = YAML::LoadFile(path);
    }
};

}

#endif //LABSTOR_SERVER_H
