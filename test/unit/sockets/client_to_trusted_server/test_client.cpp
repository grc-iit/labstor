//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/userspace_client/macros.h>
#include <labstor/userspace_client/client.h>
#include <labstor/userspace_client/ipc_manager.h>

int main() {
    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    ipc_manager_->Connect();
}