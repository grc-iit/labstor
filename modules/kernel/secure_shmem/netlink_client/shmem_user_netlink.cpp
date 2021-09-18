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

#include <labstor/types/module.h>
#include <labstor/kernel_client/kernel_client.h>

#include <secure_shmem/secure_shmem.h>
#include "shmem_user_netlink.h"

int ShmemNetlinkClient::CreateShmem(size_t region_size, bool user_owned) {
    struct shmem_request_netlink rq;
    int region_id;
    strcpy(rq.header.module_id.key, SHMEM_ID);
    rq.rq.op = RESERVE_SHMEM;
    rq.rq.reserve.size = region_size;
    rq.rq.reserve.user_owned = user_owned;
    kernel_context_->SendMSG(&rq, sizeof(rq));
    kernel_context_->RecvMSG(&region_id, sizeof(region_id));
    return region_id;
}

int ShmemNetlinkClient::GrantPidShmem(int pid, int region_id) {
    struct shmem_request_netlink rq;
    int code;
    strcpy(rq.header.module_id.key, SHMEM_ID);
    rq.rq.op = GRANT_PID_SHMEM;
    rq.rq.grant.region_id = region_id;
    kernel_context_->SendMSG(&rq, sizeof(rq));
    kernel_context_->RecvMSG(&code, sizeof(code));
    return code;
}

int ShmemNetlinkClient::FreeShmem(int region_id) {
    struct shmem_request_netlink rq;
    int code;
    strcpy(rq.header.module_id.key, SHMEM_ID);
    rq.rq.op = FREE_SHMEM;
    rq.rq.free.region_id = region_id;
    kernel_context_->SendMSG(&rq, sizeof(rq));
    kernel_context_->RecvMSG(&code, sizeof(code));
    return code;
}

void* ShmemNetlinkClient::MapShmem(size_t region_size) {
    int fd = open(shmem_chrdev_.c_str(), 0);
    if(fd < 0) {
        return nullptr;
    }
    return mmap(nullptr, region_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
}

void ShmemNetlinkClient::UnmapShmem(void *region, size_t region_size) {
    munmap(region, region_size);
}