//
// Created by lukemartinlogan on 2/9/22.
//

#include "labstor/userspace/client/client.h"
#include "labmods/registrar/client/registrar_client.h"

int main(int argc, char **argv) {
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    if(argc != 1) {
        printf("USAGE: ./terminate\n");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_ = LABSTOR_IPC_MANAGER;
    ipc_manager_->Connect();
    LABSTOR_REGISTRAR->TerminateServer();
    LABSTOR_ERROR_HANDLE_END()
}