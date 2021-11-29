//
// Created by lukemartinlogan on 11/28/21.
//

#include <labstor/userspace_server/macros.h>
#include <labstor/userspace_server/server.h>
#include <labstor/userspace_server/ipc_manager.h>
#include <labstor/userspace_server/namespace.h>
#include <labstor/userspace_server/module_manager.h>
#include <labstor/userspace_server/work_orchestrator.h>

DEFINE_SINGLETON(CONFIGURATION_MANAGER)
DEFINE_SINGLETON(IPC_MANAGER)
DEFINE_SINGLETON(MODULE_MANAGER)
DEFINE_SINGLETON(WORK_ORCHESTRATOR)
DEFINE_SINGLETON(NAMESPACE)