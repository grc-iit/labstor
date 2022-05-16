//
// Created by lukemartinlogan on 9/1/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>
#include <sys/mman.h>

#include <labstor/constants/constants.h>
#include <labstor/kernel/client/kernel_client.h>

#include <labmods/secure_shmem/secure_shmem.h>
#include "secure_shmem_client_netlink.h"

int labstor::kernel::netlink::ShmemClient::CreateShmem(size_t region_size, bool user_owned) {
    struct secure_shmem_request rq;
    int region_id;
    rq.header.ns_id_ = SHMEM_MODULE_RUNTIME_ID;
    rq.header.op_ = RESERVE_SHMEM;
    rq.reserve.size = region_size;
    rq.reserve.user_owned = user_owned;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&region_id, sizeof(region_id));
    return region_id;
}

int labstor::kernel::netlink::ShmemClient::GrantPidShmem(int pid, int region_id) {
    struct secure_shmem_request rq;
    int code;
    rq.header.ns_id_ = SHMEM_MODULE_RUNTIME_ID;
    rq.header.op_ = GRANT_PID_SHMEM;
    rq.grant.region_id = region_id;
    rq.grant.pid = pid;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&code, sizeof(code));
    return code;
}

int labstor::kernel::netlink::ShmemClient::FreeShmem(int region_id) {
    struct secure_shmem_request rq;
    int code;
    rq.header.ns_id_ = SHMEM_MODULE_RUNTIME_ID;
    rq.header.op_ = FREE_SHMEM;
    rq.free.region_id = region_id;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&code, sizeof(code));
    return code;
}

void* labstor::kernel::netlink::ShmemClient::MapShmem(int region_id, size_t region_size) {
    int fd = open(SHMEM_CHRDEV, O_RDWR);
    if(fd < 0) {
        return nullptr;
    }
    lseek(fd, region_id, SEEK_SET);
    void *data = mmap(nullptr, region_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if(data == MAP_FAILED) {
        return nullptr;
    }
    return data;
}

void labstor::kernel::netlink::ShmemClient::UnmapShmem(void *region, size_t region_size) {
    munmap(region, region_size);
}