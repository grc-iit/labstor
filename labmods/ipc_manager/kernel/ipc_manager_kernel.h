
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

#ifndef LABSTOR_IPC_MANAGER_KERNEL_KERNEL_H
#define LABSTOR_IPC_MANAGER_KERNEL_KERNEL_H

#include <labmods/ipc_manager/ipc_manager.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <linux/smp.h>

struct ipc_manager {
    int region_id;
    void *region;
    uint32_t region_size;
    struct labstor_shmem_allocator alloc;
};

extern struct ipc_manager ipc_manager_;

static inline bool ipc_manager_IsInitialized(void) {
    return labstor_shmem_allocator_GetRegion(&ipc_manager_.alloc) != 0;
}
static inline void* ipc_manager_GetRegion(void) {
    return labstor_shmem_allocator_GetRegion(&ipc_manager_.alloc);
}
static inline void* ipc_manager_Alloc(uint32_t size) {
    return labstor_shmem_allocator_Alloc(&ipc_manager_.alloc, size, smp_processor_id());
}
static inline void ipc_manager_Free(void *ptr) {
    labstor_shmem_allocator_Free(&ipc_manager_.alloc, ptr);
}

#endif //LABSTOR_IPC_MANAGER_KERNEL_KERNEL_H