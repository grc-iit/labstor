
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory>

#include <labstor/userspace/util/errors.h>
#include <labstor/constants/debug.h>
#include <labstor/constants/constants.h>
#include "ipc_manager_client_netlink.h"

void* labstor::kernel::netlink::IPCManager::Register(int region_id) {
    void *region;
    struct labstor_ipc_manager_register_request rq;
    rq.header.ns_id_ = IPC_MANAGER_MODULE_RUNTIME_ID;
    rq.header.op_ = IPC_MANAGER_REGISTER;
    rq.region_id = region_id;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&region, sizeof(region));
    if(region == 0) {
        throw KERNEL_IPC_MANAGER_FAILED_TO_REGISTER.format(0);
    }
    return region;
}

void labstor::kernel::netlink::IPCManager::Unregister() {
    //TODO: Unregister IPC manager
}