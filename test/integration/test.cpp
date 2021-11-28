//
// Created by lukemartinlogan on 11/26/21.
//

#include <labstor/userspace_client/client.h>
#include "dummy/client/dummy_client.h"

#include <unistd.h>

int main() {
    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    labstor::test::Dummy::Client client;
    printf("PID: %d\n", getpid());
    ipc_manager_->Connect();
    //client.Register();
    //client.GetValue();
}