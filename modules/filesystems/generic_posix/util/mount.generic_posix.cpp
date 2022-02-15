//
// Created by lukemartinlogan on 12/30/21.
//

#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <modules/registrar/registrar.h>
#include <modules/registrar/client/registrar_client.h>
#include <labstor/userspace/util/error.h>
#include <generic_posix.h>

int main(int argc, char **argv) {
    if(argc != 1) {
        printf("USAGE: ./mount.generic_posix");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        auto registrar = labstor::Registrar::Client();
        ipc_manager_->Connect();
        int ns_id = registrar.RegisterInstance<labstor::Registrar::register_request>(GENERIC_POSIX_MODULE_ID, GENERIC_POSIX_MODULE_ID);
        printf("Mounted FS %s on %s with namespace ID %d\n", GENERIC_POSIX_MODULE_ID, GENERIC_POSIX_MODULE_ID, ns_id);
    LABSTOR_ERROR_HANDLE_END()
}