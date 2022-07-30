
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

#ifndef LABSTOR_CLIENT_MACROS_H
#define LABSTOR_CLIENT_MACROS_H

#include <labstor/constants/macros.h>
#include <labstor/userspace/util/singleton.h>
#define LABSTOR_CONFIGURATION_MANAGER_CLASS labstor::Client::ConfigurationManager
#define LABSTOR_IPC_MANAGER_CLASS labstor::Client::IPCManager
#define LABSTOR_MODULE_MANAGER_CLASS labstor::Client::ModuleManager
#define LABSTOR_WORK_ORCHESTRATOR_CLASS labstor::Client::WorkOrchestrator
#define LABSTOR_NAMESPACE_CLASS labstor::Client::Namespace
#define LABSTOR_REGISTRAR_CLASS labstor::Registrar::Client

#define LABSTOR_CONFIGURATION_MANAGER_T SINGLETON_T(LABSTOR_CONFIGURATION_MANAGER_CLASS)
#define LABSTOR_IPC_MANAGER_T SINGLETON_T(LABSTOR_IPC_MANAGER_CLASS)
#define LABSTOR_MODULE_MANAGER_T SINGLETON_T(LABSTOR_MODULE_MANAGER_CLASS)
#define LABSTOR_WORK_ORCHESTRATOR_T SINGLETON_T(LABSTOR_WORK_ORCHESTRATOR_CLASS)
#define LABSTOR_NAMESPACE_T SINGLETON_T(LABSTOR_NAMESPACE_CLASS)
#define LABSTOR_REGISTRAR_T SINGLETON_T(LABSTOR_REGISTRAR_CLASS)

#define LABSTOR_CONFIGURATION_MANAGER_SINGLETON scs::Singleton<LABSTOR_CONFIGURATION_MANAGER_CLASS>
#define LABSTOR_IPC_MANAGER_SINGLETON scs::Singleton<LABSTOR_IPC_MANAGER_CLASS>
#define LABSTOR_MODULE_MANAGER_SINGLETON scs::Singleton<LABSTOR_MODULE_MANAGER_CLASS>
#define LABSTOR_WORK_ORCHESTRATOR_SINGLETON scs::Singleton<LABSTOR_WORK_ORCHESTRATOR_CLASS>
#define LABSTOR_NAMESPACE_SINGLETON scs::Singleton<LABSTOR_NAMESPACE_CLASS>
#define LABSTOR_REGISTRAR_SINGLETON scs::Singleton<LABSTOR_REGISTRAR_CLASS>

#define LABSTOR_CONFIGURATION_MANAGER LABSTOR_CONFIGURATION_MANAGER_SINGLETON::GetInstance()
#define LABSTOR_IPC_MANAGER LABSTOR_IPC_MANAGER_SINGLETON::GetInstance()
#define LABSTOR_MODULE_MANAGER LABSTOR_MODULE_MANAGER_SINGLETON::GetInstance()
#define LABSTOR_WORK_ORCHESTRATOR LABSTOR_WORK_ORCHESTRATOR_SINGLETON::GetInstance()
#define LABSTOR_NAMESPACE LABSTOR_NAMESPACE_SINGLETON::GetInstance()
#define LABSTOR_REGISTRAR LABSTOR_REGISTRAR_SINGLETON::GetInstance()

#endif //LABSTOR_CLIENT_MACROS_H