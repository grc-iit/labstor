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

#include <workers/worker_kernel.h>
#include "worker_user_netlink.h"

int WorkerNetlinkClient::CreateWorkers(int num_workers, int region_id, size_t region_size, size_t time_slice_us) {
    struct kernel_worker_request_netlink rq;
    strcpy(rq.header.module_id.key, WORKER_MODULE_ID);
    rq.rq.op = SPAWN_WORKERS;
    rq.rq.spawn.num_workers = num_workers;
    rq.rq.spawn.region_id = region_id;
    rq.rq.spawn.region_size = region_size;
    rq.rq.spawn.time_slice_us = time_slice_us;
    kernel_context_->SendMSG(&rq, sizeof(rq));
    kernel_context_->RecvMSG(&region_id, sizeof(region_id));
    return region_id;
}

int WorkerNetlinkClient::SetAffinity(int worker_id, int cpu_id) {
    struct kernel_worker_request_netlink rq;
    int code;
    strcpy(rq.header.module_id.key, WORKER_MODULE_ID);
    rq.rq.op = SET_WORKER_AFFINITY;
    rq.rq.affinity.worker_id = worker_id;
    rq.rq.affinity.cpu_id = cpu_id;
    kernel_context_->SendMSG(&rq, sizeof(rq));
    kernel_context_->RecvMSG(&code, sizeof(code));
    return code;
}
