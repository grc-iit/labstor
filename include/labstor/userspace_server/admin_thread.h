//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_ADMIN_THREAD_H
#define LABSTOR_ADMIN_THREAD_H

#include <labstor/util/errors.h>
#include <labstor/types/daemon/daemon.h>
#include <labstor/userspace_server/ipc_manager.h>
#include <labstor/types/messages.h>

namespace labstor::Server {

class AdminWorker : public DaemonWorker {
private:
    int nothing;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    AdminWorker() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    void DoWork() override {
        int op;
        for(int pid : ipc_manager_->GetConnectedProcesses()) {
            LABSTOR_ERROR_HANDLE_TRY {
                labstor::ipc::admin_request header;
                PerProcessIPC *ipc = ipc_manager_->GetIPC(pid);
                if(!ipc->GetSocket().RecvMSGPeek(&header, sizeof(header), false)) {
                    continue;
                }
            } LABSTOR_ERROR_HANDLE_CATCH {
                //TODO: Free IPC
            }
        }
    }
};

}

#endif //LABSTOR_ADMIN_THREAD_H
