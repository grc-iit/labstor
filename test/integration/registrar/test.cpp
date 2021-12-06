//
// Created by lukemartinlogan on 11/26/21.
//

#include <labstor/userspace/client/client.h>
#include <modules/dummy/client/dummy_client.h>

#include <unistd.h>

int main() {
    LABSTOR_ERROR_HANDLE_START()

    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    labstor::test::Dummy::Client client;
    printf("PID: %d\n", getpid());
    ipc_manager_->Connect();
    client.Register();

    for(int i = 0; i < 2048; ++i) {
        client.GetValue();
    }

    LABSTOR_ERROR_HANDLE_END()
}