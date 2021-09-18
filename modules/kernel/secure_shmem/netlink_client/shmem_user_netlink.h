//
// Created by lukemartinlogan on 9/17/21.
//

#ifndef LABSTOR_SHMEM_USER_NETLINK_H
#define LABSTOR_SHMEM_USER_NETLINK_H

#include <labstor/util/singleton.h>
#include <labstor/kernel_client/kernel_client.h>

class ShmemNetlinkClient {
private:
    std::shared_ptr<labstor::LabStorKernelClientContext> kernel_context_;
    std::string shmem_chrdev_;
public:
    ShmemNetlinkClient() {
        kernel_context_ = scs::Singleton<labstor::LabStorKernelClientContext>::GetInstance();
        shmem_chrdev_ = "/proc/devices/labstor_shmem";
    }
    int CreateShmem(size_t region_size, bool user_owned);
    int GrantPidShmem(int pid, int region_id);
    int FreeShmem(int region_id);
    void* MapShmem(size_t region_size);
    void UnmapShmem(void *region, size_t region_size);
};

#endif //LABSTOR_SHMEM_USER_NETLINK_H
