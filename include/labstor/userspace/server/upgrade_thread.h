
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