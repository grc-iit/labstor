//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_SERVER_MACROS_H
#define LABSTOR_SERVER_MACROS_H

#include <labstor/util/singleton.h>
#define LABSTOR_CONFIGURATION_MANAGER_CLASS labstor::Server::ConfigurationManager
#define LABSTOR_IPC_MANAGER_CLASS labstor::Server::IPCManager
#define LABSTOR_MODULE_MANAGER_CLASS labstor::Server::ModuleManager
#define LABSTOR_WORK_ORCHESTRATOR_CLASS labstor::Server::WorkOrchestrator
#define LABSTOR_NAMESPACE_CLASS labstor::Server::Namespace
#define LABSTOR_KERNEL_CLIENT_CLASS labstor::Kernel::NetlinkClient

#define LABSTOR_CONFIGURATION_MANAGER_T std::shared_ptr<LABSTOR_CONFIGURATION_MANAGER_CLASS>
#define LABSTOR_IPC_MANAGER_T std::shared_ptr<LABSTOR_IPC_MANAGER_CLASS>
#define LABSTOR_MODULE_MANAGER_T std::shared_ptr<LABSTOR_MODULE_MANAGER_CLASS>
#define LABSTOR_WORK_ORCHESTRATOR_T std::shared_ptr<LABSTOR_WORK_ORCHESTRATOR_CLASS>
#define LABSTOR_NAMESPACE_T std::shared_ptr<LABSTOR_NAMESPACE_CLASS>
#define LABSTOR_KERNEL_CLIENT_T std::shared_ptr<LABSTOR_KERNEL_CLIENT_CLASS>

#define LABSTOR_CONFIGURATION_MANAGER scs::Singleton<LABSTOR_CONFIGURATION_MANAGER_CLASS>::GetInstance()
#define LABSTOR_IPC_MANAGER scs::Singleton<LABSTOR_IPC_MANAGER_CLASS>::GetInstance()
#define LABSTOR_MODULE_MANAGER scs::Singleton<LABSTOR_MODULE_MANAGER_CLASS>::GetInstance()
#define LABSTOR_WORK_ORCHESTRATOR scs::Singleton<LABSTOR_WORK_ORCHESTRATOR_CLASS>::GetInstance()
#define LABSTOR_NAMESPACE scs::Singleton<LABSTOR_NAMESPACE_CLASS>::GetInstance()
#define LABSTOR_KERNEL_CLIENT scs::Singleton<LABSTOR_KERNEL_CLIENT_CLASS>::GetInstance()

#endif //LABSTOR_SERVER_MACROS_H
