//
// Created by lukemartinlogan on 2/9/22.
//

#include "labstor/userspace/client/client.h"
#include <labmods/iosched/no_op/client/no_op_client.h>

int main(int argc, char **argv) {
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    labstor::iosched::NoOp::Client client_;
    if(argc != 1) {
        printf("USAGE: ./register [ns_key] [dev]\n");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_ = LABSTOR_IPC_MANAGER;
    ipc_manager_->Connect();
    client_.Register(argv[1], argv[2]);
    LABSTOR_ERROR_HANDLE_END()
}