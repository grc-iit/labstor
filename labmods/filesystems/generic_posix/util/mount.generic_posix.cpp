//
// Created by lukemartinlogan on 12/30/21.
//

#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labmods/registrar/registrar.h>
#include <labmods/registrar/client/registrar_client.h>
#include <labstor/userspace/util/error.h>
#include <generic_posix.h>
#include "client/generic_posix_client.h"

int main(int argc, char **argv) {
    if(argc != 1) {
        printf("USAGE: ./mount.generic_posix");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        auto client = labstor::GenericPosix::Client();
        ipc_manager_->Connect();
        int ns_id = client.Register();
        printf("GenericPOSIX: Mounted (ns_id=%d)\n", GENERIC_POSIX_MODULE_ID, ns_id);
    LABSTOR_ERROR_HANDLE_END()
}