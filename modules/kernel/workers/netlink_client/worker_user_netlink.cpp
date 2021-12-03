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

#include <labstor/kernel/constants/runtime_ids.h>
#include <labstor/kernel/client/kernel_client.h>

#include <workers/worker_kernel.h>
#include "worker_user_netlink.h"

int WorkerNetlinkClient::CreateWorkers(int num_workers, int region_id, size_t region_size, size_t time_slice_us) {
    struct kernel_worker_request_netlink rq;
    rq.header.runtime_id_ = WORKER_MODULE_RUNTIME_ID;
    rq.rq.header.op_ = SPAWN_WORKERS;
    rq.rq.spawn.num_workers = num_workers;
    rq.rq.spawn.region_id = region_id;
    rq.rq.spawn.region_size = region_size;
    rq.rq.spawn.time_slice_us = time_slice_us;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&region_id, sizeof(region_id));
    return region_id;
}

int WorkerNetlinkClient::SetAffinity(int worker_id, int cpu_id) {
    struct kernel_worker_request_netlink rq;
    int code;
    rq.header.runtime_id_ = WORKER_MODULE_RUNTIME_ID;
    rq.rq.header.op_ = SET_WORKER_AFFINITY;
    rq.rq.affinity.worker_id = worker_id;
    rq.rq.affinity.cpu_id = cpu_id;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&code, sizeof(code));
    return code;
}
