//
// Created by lukemartinlogan on 5/15/22.
//

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
