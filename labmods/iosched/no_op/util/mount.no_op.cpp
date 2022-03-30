//
// Created by lukemartinlogan on 2/9/22.
//

#include "labstor/userspace/client/client.h"
#include <labmods/iosched/no_op/client/no_op_client.h>

int main(int argc, char **argv) {
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    labstor::iosched::NoOp::Client client_;
    if(argc != 3) {
        printf("USAGE: ./register [ns_key] [next_module (GenericQueue)]\n");
        exit(1);
    }
    char *ns_key = argv[1];
    char *next_module = argv[2];
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_ = LABSTOR_IPC_MANAGER;
    ipc_manager_->Connect();
    client_.Register(ns_key, next_module);
    printf("NoOp: Mounted %s (next=%s, ns_id=%s)\n", ns_key, next_module, client_.GetNamespaceID());
    LABSTOR_ERROR_HANDLE_END()
}