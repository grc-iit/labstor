//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_SERVER_H
#define LABSTOR_SERVER_H

#include <labstor/types/ipc_pool.h>
#include <labstor/types/package.h>
#include <labstor/ipc/unordered_map.h>
#include <labstor/userspace_server/ipc_manager.h>

#include <pthread.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace labstor {

struct LabStorServerContext {
    int pid_;
    labstor::ipc::unordered_map<std::string, labstor::package> namespace_;
    std::unordered_map <pid_t, std::shared_ptr<labstor::PerProcessIPC>> ipc_pool_;
    labstor::PackagePool pkg_pool_;
    labstor::IPCManager ipc_manager_;
    pthread_t work_orchestrator_;
    std::vector<pthread_t> worker_pool_;
    pthread_t accept_thread_;
    int accept_fd_;
};

}

#endif //LABSTOR_SERVER_H
