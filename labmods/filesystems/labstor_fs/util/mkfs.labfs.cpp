//
// Created by lukemartinlogan on 8/17/21.
//

#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/util/error.h>
#include <labstor/userspace/client/namespace.h>
#include <labmods/registrar/registrar.h>
#include <labmods/registrar/client/registrar_client.h>
#include <labmods/storage_api/generic_block/generic_block.h>
#include <labmods/storage_api/generic_block/client/generic_block_client.h>
#include <labmods/filesystems/labstor_fs/lib/labstor_fs_log.h>
#include <labmods/filesystems/labstor_fs/client/labstor_fs_client.h>

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("USAGE: ./mkfs.labfs [block-dev-key]");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        LABSTOR_NAMESPACE_T namespace_ = LABSTOR_NAMESPACE;
        ipc_manager_->Connect();
        uint32_t ns_id = namespace_->Get(argv[1]);
        labstor::GenericBlock::Client *block_dev = namespace_->LoadClientModule<labstor::GenericBlock::Client>(ns_id);
        void *buf = calloc(SMALL_BLOCK_SIZE, 0);
        block_dev->Write(buf, SMALL_BLOCK_SIZE, 0);
    LABSTOR_ERROR_HANDLE_END()
}