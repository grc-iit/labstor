//
// Created by lukemartinlogan on 11/28/21.
//

#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/server/work_orchestrator.h>

DEFINE_SINGLETON(CONFIGURATION_MANAGER)
DEFINE_SINGLETON(IPC_MANAGER)
DEFINE_SINGLETON(MODULE_MANAGER)
DEFINE_SINGLETON(WORK_ORCHESTRATOR)
DEFINE_SINGLETON(NAMESPACE)