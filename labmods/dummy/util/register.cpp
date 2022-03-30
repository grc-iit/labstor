//
// Created by lukemartinlogan on 2/9/22.
//

#include "labstor/userspace/client/client.h"
#include "client/dummy_client.h"

int main(int argc, char **argv) {
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    labstor::test::Dummy::Client client;
    if(argc != 1) {
        printf("USAGE: ./register\n");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_ = LABSTOR_IPC_MANAGER;
    ipc_manager_->Connect();
    client.Register();
    printf("Dummy: registered (ns_id=%d)\n", client.GetNamespaceID());
    LABSTOR_ERROR_HANDLE_END()
}