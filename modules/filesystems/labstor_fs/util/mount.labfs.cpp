//
// Created by lukemartinlogan on 8/17/21.
//

#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <modules/registrar/registrar.h>
#include <modules/registrar/client/registrar_client.h>
#include <labstor/userspace/util/error.h>
#include <modules/filesystems/labstor_fs/labstor_fs.h>
#include <modules/filesystems/labstor_fs/client/labstor_fs_client.h>

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("USAGE: ./mount.labfs [namespace-key] [next_module]");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        auto labfs = labstor::LabFS::Client();
        ipc_manager_->Connect();
        labfs.Register(argv[1], argv[2]);
        printf("Mounted FS %s on %s with namespace ID?\n", LABFS_MODULE_ID, argv[1]);
    LABSTOR_ERROR_HANDLE_END()
}