
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

#ifndef LABSTOR_KERNEL_CLIENT_MACROS_H
#define LABSTOR_KERNEL_CLIENT_MACROS_H

#include <labstor/userspace/util/singleton.h>
#define LABSTOR_KERNEL_CLIENT_CLASS labstor::Kernel::NetlinkClient
#define LABSTOR_KERNEL_IPC_MANAGER_CLASS labstor::kernel::netlink::IPCManager
#define LABSTOR_KERNEL_WORK_ORCHESTRATOR_CLASS labstor::kernel::netlink::WorkerClient
#define LABSTOR_KERNEL_SHMEM_ALLOC_CLASS labstor::kernel::netlink::ShmemClient

#define LABSTOR_KERNEL_CLIENT_T SINGLETON_T(LABSTOR_KERNEL_CLIENT_CLASS)
#define LABSTOR_KERNEL_IPC_MANAGER_T SINGLETON_T(LABSTOR_KERNEL_IPC_MANAGER_CLASS)
#define LABSTOR_KERNEL_WORK_ORCHESTRATOR_T SINGLETON_T(LABSTOR_KERNEL_WORK_ORCHESTRATOR_CLASS)
#define LABSTOR_KERNEL_SHMEM_ALLOC_T SINGLETON_T(LABSTOR_KERNEL_SHMEM_ALLOC_CLASS)

#define LABSTOR_KERNEL_CLIENT_SINGLETON scs::Singleton<LABSTOR_KERNEL_CLIENT_CLASS>
#define LABSTOR_KERNEL_IPC_MANAGER_SINGLETON scs::Singleton<LABSTOR_KERNEL_IPC_MANAGER_CLASS>
#define LABSTOR_KERNEL_WORK_ORCHESTRATOR_SINGLETON scs::Singleton<LABSTOR_KERNEL_WORK_ORCHESTRATOR_CLASS>
#define LABSTOR_KERNEL_SHMEM_ALLOC_SINGLETON scs::Singleton<LABSTOR_KERNEL_SHMEM_ALLOC_CLASS>

#define LABSTOR_KERNEL_CLIENT LABSTOR_KERNEL_CLIENT_SINGLETON::GetInstance()
#define LABSTOR_KERNEL_IPC_MANAGER LABSTOR_KERNEL_IPC_MANAGER_SINGLETON::GetInstance()
#define LABSTOR_KERNEL_WORK_ORCHESTRATOR LABSTOR_KERNEL_WORK_ORCHESTRATOR_SINGLETON::GetInstance()
#define LABSTOR_KERNEL_SHMEM_ALLOC LABSTOR_KERNEL_SHMEM_ALLOC_SINGLETON::GetInstance()

#endif //LABSTOR_KERNEL_CLIENT_MACROS_H