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
    if(argc != 3) {
        printf("USAGE: ./mount.labfs [ns-key] [next_module]");
        exit(1);
    }
    char *ns_key = argv[1];
    char *next_module = argv[2];
    LABSTOR_ERROR_HANDLE_START()
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        auto labfs = labstor::BlockFS::Client();
        ipc_manager_->Connect();
        labfs.Register(ns_key, argv[2]);
        printf("BlockFS: Mounted %s, pointing to %s?\n",ns_key, next_module);
    LABSTOR_ERROR_HANDLE_END()
}