//
// Created by lukemartinlogan on 9/16/21.
//

#ifndef LABSTOR_IPC_MANAGER_KERNEL_H
#define LABSTOR_IPC_MANAGER_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>

#define IPC_MANAGER_MODULE_ID "IPC_MANAGER"

enum {
    IPC_MANAGER_REGISTER,
    IPC_MANAGER_UNREGISTER
};

struct labstor_ipc_manager_register_request {
    struct labstor_request header;
    int region_id;
};

#endif //LABSTOR_IPC_MANAGER_KERNEL_H
