//
// Created by lukemartinlogan on 9/1/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory>

#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/util/debug.h>
#include <labstor/constants/constants.h>
#include "ipc_manager_client_netlink.h"

void labstor::kernel::netlink::IPCManager::Register(int region_id) {
    AUTO_TRACE("labstor::kernel::netlink::IPCManager::Register", region_id);
    int code;
    struct labstor_ipc_manager_register_request rq;
    rq.header.ns_id_ = IPC_MANAGER_MODULE_RUNTIME_ID;
    rq.header.op_ = IPC_MANAGER_REGISTER;
    rq.region_id = region_id;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    TRACEPOINT("SendMsg");
    kernel_client_->RecvMSG(&code, sizeof(code));
    TRACEPOINT("RecvMsg");
    if(code != 0) {
        throw KERNEL_IPC_MANAGER_FAILED_TO_REGISTER.format(code);
    }
}

void labstor::kernel::netlink::IPCManager::Unregister() {
    //TODO: Unregister IPC manager
}
