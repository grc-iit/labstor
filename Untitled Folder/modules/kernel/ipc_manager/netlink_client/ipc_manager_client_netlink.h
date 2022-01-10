//
// Created by lukemartinlogan on 9/17/21.
//

#ifndef LABSTOR_IPC_MANAGER_CLIENT_NETLINK_H
#define LABSTOR_IPC_MANAGER_CLIENT_NETLINK_H

#include <labstor/kernel/client/macros.h>
#include <labstor/kernel/client/kernel_client.h>
#include <modules/kernel/ipc_manager/ipc_manager.h>

#define IPC_MANAGER_MODULE_ID "IPC_MANAGER"

namespace labstor::kernel::netlink {

class IPCManager {
private:
    LABSTOR_KERNEL_CLIENT_T kernel_client_;
public:
    IPCManager() {
        kernel_client_ = LABSTOR_KERNEL_CLIENT;
    }
    void *Register(int region_id);
    void Unregister(void);
};

}

#endif //LABSTOR_IPC_MANAGER_CLIENT_NETLINK_H
