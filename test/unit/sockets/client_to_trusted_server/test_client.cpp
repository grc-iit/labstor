//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/ipc_manager.h>

int main() {
    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    ipc_manager_->Connect();
}