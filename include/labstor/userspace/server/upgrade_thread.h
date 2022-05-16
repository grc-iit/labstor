//
// Created by lukemartinlogan on 5/15/22.
//

#ifndef LABSTOR_UPGRADE_THREAD_H
#define LABSTOR_UPGRADE_THREAD_H

#include <labstor/userspace/util/errors.h>
#include <labstor/types/daemon.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/types/messages.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/util/timer.h>

namespace labstor::Server {

class UpgradeWorker : public DaemonWorker {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_MODULE_MANAGER_T module_manager_;
    labstor::HighResMonotonicTimer t_;
    int sec_;
public:
    UpgradeWorker() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        module_manager_ = LABSTOR_MODULE_MANAGER;
        sec_ = 5;
        t_.Resume();
    }

    void DoWork() override {
        t_.Pause();
        if(t_.GetSec() >= sec_) {
            for(auto &upgrade : module_manager_->GetUpgrades()) {
                YAML::Node config = YAML::LoadFile(upgrade->yaml_path_);
                if(config["code_update"]) {
                    std::string update_type = config["code_update"].as<std::string>();
                    std::string labmod_uuid = config["labmod_uuid"].as<std::string>();
                    if(config["code_update"]["centralized"]) {
                        module_manager_->CentralizedUpdateModule(config);
                    }
                    if(config["code_update"]["decentralized"]) {
                        module_manager_->DecentralizedUpdateModule(config);
                    }
                }
            }
            t_.Reset();
        }
        t_.Resume();
        sched_yield();
    }
};

}

#endif //LABSTOR_UPGRADE_THREAD_H
