//
// Created by lukemartinlogan on 11/26/21.
//

#include <labstor/userspace/client/client.h>
#include <modules/kernel/ipc_test/client/ipc_test_client.h>

#include <unistd.h>

int main() {
    LABSTOR_ERROR_HANDLE_START()

    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    labstor::IPCTest::Client client;
    printf("PID: %d\n", getpid());
    ipc_manager_->Connect();
    client.Register();

    for(int i = 0; i < 1; ++i) {
        client.Start();
    }

    LABSTOR_ERROR_HANDLE_END()
}