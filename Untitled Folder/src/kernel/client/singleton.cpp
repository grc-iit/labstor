//
// Created by lukemartinlogan on 11/28/21.
//

#include <labstor/constants/macros.h>
#include <labstor/kernel/client/macros.h>
#include <labstor/kernel/client/kernel_client.h>
#include <modules/kernel/ipc_manager/netlink_client/ipc_manager_client_netlink.h>
#include <modules/kernel/work_orchestrator/netlink_client/work_orchestrator_client_netlink.h>
#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

DEFINE_SINGLETON(KERNEL_CLIENT)
DEFINE_SINGLETON(KERNEL_IPC_MANAGER)
DEFINE_SINGLETON(KERNEL_WORK_ORCHESTRATOR)
DEFINE_SINGLETON(KERNEL_SHMEM_ALLOC)