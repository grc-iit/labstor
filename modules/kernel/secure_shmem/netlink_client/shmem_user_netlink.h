//
// Created by lukemartinlogan on 9/17/21.
//

#ifndef LABSTOR_SHMEM_USER_NETLINK_H
#define LABSTOR_SHMEM_USER_NETLINK_H

#include <labstor/kernel_client/macros.h>
#include <labstor/kernel_client/kernel_client.h>

#define SHMEM_CHRDEV "/dev/labstor_shared_shmem0"

class ShmemNetlinkClient {
private:
    LABSTOR_KERNEL_CLIENT_T kernel_client_;
    int PAGE_SIZE;
public:
    ShmemNetlinkClient() {
        kernel_client_ = LABSTOR_KERNEL_CLIENT;
        PAGE_SIZE = getpagesize();
    }
    int CreateShmem(size_t region_size, bool user_owned);
    int GrantPidShmem(int pid, int region_id);
    int FreeShmem(int region_id);
    static void* MapShmem(int region_id, size_t region_size);
    static void UnmapShmem(void *region, size_t region_size);
};

#endif //LABSTOR_SHMEM_USER_NETLINK_H
