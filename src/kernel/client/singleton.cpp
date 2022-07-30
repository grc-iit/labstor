
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

#include <labstor/constants/macros.h>
#include <labstor/kernel/client/macros.h>
#include <labstor/kernel/client/kernel_client.h>
#include <labmods/ipc_manager/netlink_client/ipc_manager_client_netlink.h>
#include <labmods/work_orchestrator/netlink_client/work_orchestrator_client_netlink.h>
#include <labmods/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

DEFINE_SINGLETON(KERNEL_CLIENT)
DEFINE_SINGLETON(KERNEL_IPC_MANAGER)
DEFINE_SINGLETON(KERNEL_WORK_ORCHESTRATOR)
DEFINE_SINGLETON(KERNEL_SHMEM_ALLOC)