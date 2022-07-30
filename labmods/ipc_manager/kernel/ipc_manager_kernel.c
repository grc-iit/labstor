
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

/*
 * Provides shared memory between the kernel and a process.
 * */

#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/c/shmem_queue_pair.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>

#include <labmods/secure_shmem/kernel/secure_shmem_kernel.h>
#include <labmods/ipc_manager/kernel/ipc_manager_kernel.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module for managing shared memory between kernel and client");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipc_manager");

struct ipc_manager ipc_manager_ = {0};
EXPORT_SYMBOL(ipc_manager_);

inline void ipc_manager_register(struct labstor_ipc_manager_register_request *rq) {
    ipc_manager_.region = labstor_find_shmem_region(rq->region_id);
    if(ipc_manager_.region == NULL) {
        pr_warn("IPC Manager could not find region %d\n", rq->region_id);
        return;
    }
    pr_info("IPCManager region: %p\n", ipc_manager_.region);
    ipc_manager_.region_id = rq->region_id;
    labstor_shmem_allocator_Attach(&ipc_manager_.alloc, ipc_manager_.region, ipc_manager_.region);
}

void ipc_manager_process_request_fn_netlink(int pid, struct labstor_request *rq) {
    int code = 0;
    switch(rq->op_) {
        case IPC_MANAGER_REGISTER: {
            pr_info("Creating IPC manager\n");
            ipc_manager_register((struct labstor_ipc_manager_register_request *)rq);
            labstor_msg_trusted_server(&ipc_manager_.region, sizeof(void*), pid);
            break;
        }

        case IPC_MANAGER_UNREGISTER: {
            pr_warn("Unregister not yet implemented\n");
            code = -1;
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        default: {
            pr_warn("Invalid request id: %d\n", rq->op_);
            code = -1;
            labstor_msg_trusted_server(&code, sizeof(code), pid);
        }
    }
}

struct labstor_module shmem_module = {
    .module_id = IPC_MANAGER_MODULE_ID,
    .runtime_id = IPC_MANAGER_MODULE_RUNTIME_ID,
    .process_request_fn = NULL,
    .process_request_fn_netlink = (process_request_fn_netlink_type)ipc_manager_process_request_fn_netlink,
};

static int __init init_ipc_manager(void) {
    register_labstor_module(&shmem_module);
    pr_info("IPC manager has started");
    return 0;
}

static void __exit exit_ipc_manager(void) {
    unregister_labstor_module(&shmem_module);
    pr_info("IPC manager has ended");
}

module_init(init_ipc_manager)
module_exit(exit_ipc_manager)