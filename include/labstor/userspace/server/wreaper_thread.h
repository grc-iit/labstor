
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

#ifndef LABSTOR_WREAPER_THREAD_H
#define LABSTOR_WREAPER_THREAD_H

#include <labstor/userspace/util/errors.h>
#include <labstor/types/daemon.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/types/messages.h>

namespace labstor::Server {

class WreaperWorker : public DaemonWorker {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    WreaperWorker() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    void DoWork() override {
        int op;
        auto &ipcs = ipc_manager_->GetAllIPC();
        for(auto iter = ipcs.begin(); iter != ipcs.end();) {
            LABSTOR_ERROR_HANDLE_TRY {
                labstor::ipc::admin_request header;
                PerProcessIPC *ipc = iter->second;
                if(ipc->GetPID() == ipc_manager_->GetPID()) { continue; }
                if(ipc->GetPID() == 0) { continue; }
                if(!ipc->GetSocket().RecvMSGPeek(&header, sizeof(header), false)) {
                    continue;
                }
                ++iter;
            } LABSTOR_ERROR_HANDLE_CATCH {
                printf("PID %d disconnected\n", iter->first);
                delete iter->second;
                iter = ipcs.erase(iter);
            }
        }
    }
};

}

#endif //LABSTOR_WREAPER_THREAD_H