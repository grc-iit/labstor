//
// Created by lukemartinlogan on 12/4/21.
//

#ifndef LABSTOR_IPC_MANAGER_KERNEL_KERNEL_H
#define LABSTOR_IPC_MANAGER_KERNEL_KERNEL_H

#include <ipc_manager/ipc_manager.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <linux/smp.h>

struct ipc_manager {
    int region_id;
    void *region;
    uint32_t region_size;
    struct labstor_shmem_allocator alloc;
};

extern struct ipc_manager ipc_manager_;

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
