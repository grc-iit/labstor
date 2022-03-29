//
// Created by lukemartinlogan on 8/17/21.
//

#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labmods/registrar/registrar.h>
#include <labmods/registrar/client/registrar_client.h>
#include <labstor/userspace/util/error.h>
#include <labmods/filesystems/block_fs/block_fs.h>
#include <labmods/filesystems/block_fs/client/block_fs_client.h>

int main(int argc, char **argv) {
    if(argc != 4) {
        printf("USAGE: ./mount.labfs [namespace-key] [next_module] [has_fs]");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        auto labfs = labstor::BlockFS::Client();
        ipc_manager_->Connect();
        labfs.Register(argv[1], argv[2]);
        printf("Mounted FS %s on %s with namespace ID?\n", BLOCKFS_MODULE_ID, argv[1]);
    LABSTOR_ERROR_HANDLE_END()
}