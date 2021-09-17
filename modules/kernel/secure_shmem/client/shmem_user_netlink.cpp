//
// Created by lukemartinlogan on 9/1/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <memory>

#include <labstor/types/module.h>
#include <labstor/kernel_client/kernel_client.h>

#include "secure_shmem.h"

class ShmemNetlinkClient {
private:
    std::shared_ptr<scs::LabStorKernelClientContext> kernel_context_;
public:
    ShmemNetlinkClient() {
        kernel_context_ = Singleton<scs::LabStorKernelClientContext>::GetInstance();
    }

    int CreateShmem(size_t region_size, bool user_owned) {
        struct shmem_request rq;
        int region_id;
        rq.op = RESERVE_SHMEM;
        rq.reserve.size = region_size;
        rq.reserve.user_owned = user_owned;
        kernel_context_->SendMSG(&rq, sizeof(rq));
        kernel_context_->RecvMSG(&region_id, sizeof(region_id));
        return region_id;
    }

    int GrantPidShmem(int pid, int region_id) {
        struct shmem_request rq;
        int code;
        rq.op = GRANT_PID_SHMEM;
        rq.grant.region_id = region_size;
        kernel_context_->SendMSG(&rq, sizeof(rq));
        kernel_context_->RecvMSG(&code, sizeof(code));
        return code;
    }

    void FreeShmem() {
        struct shmem_request rq;
        int code;
        rq.op = FREE_SHMEM;
        rq.free.region_id = region_size;
        kernel_context_->SendMSG(&rq, sizeof(rq));
        kernel_context_->RecvMSG(&code, sizeof(code));
        return code;
    }

    void* GetShmem(size_t region_size) {
    }

    static struct shmem_ops* get_ops() { return &ops_; }
};