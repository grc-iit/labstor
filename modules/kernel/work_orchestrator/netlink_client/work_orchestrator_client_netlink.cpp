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

#include <work_orchestrator/work_orchestrator.h>
#include "work_orchestrator_client_netlink.h"

int labstor::kernel::netlink::WorkerClient::CreateWorkers(int num_workers, int region_id, size_t region_size, size_t time_slice_us) {
    int code;
    struct labstor_spawn_worker_request rq;
    rq.header.ns_id_ = WORKER_MODULE_RUNTIME_ID;
    rq.header.op_ = LABSTOR_SPAWN_WORKERS;
    rq.num_workers = num_workers;
    rq.region_id = region_id;
    rq.region_size = region_size;
    rq.time_slice_us = time_slice_us;
    num_workers_ = num_workers;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&code, sizeof(code));
    return region_id;
}

void labstor::kernel::netlink::WorkerClient::AssignQueuePair(labstor::ipc::queue_pair *qp, void *base) {
    int code;
    struct labstor_assign_queue_pair_request rq;
    rq.header.ns_id_ = WORKER_MODULE_RUNTIME_ID;
    rq.header.op_ = LABSTOR_ASSIGN_QP;
    qp->GetPointer(rq.ptr, base);
    rq.worker_id = qp->GetQid() % num_workers_;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&code, sizeof(code));
}

void labstor::kernel::netlink::WorkerClient::SetAffinity(int cpu_id) {
    struct labstor_set_worker_affinity_request rq;
    int code;
    rq.header.ns_id_ = WORKER_MODULE_RUNTIME_ID;
    rq.header.op_ = LABSTOR_SET_WORKER_AFFINITY;
    rq.worker_id = worker_id_;
    rq.cpu_id = cpu_id;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&code, sizeof(code));
}

void labstor::kernel::netlink::WorkerClient::Start() {}
void labstor::kernel::netlink::WorkerClient::Stop() {}
void labstor::kernel::netlink::WorkerClient::Pause() {}
void labstor::kernel::netlink::WorkerClient::Resume() {}
void labstor::kernel::netlink::WorkerClient::Wait() {}
