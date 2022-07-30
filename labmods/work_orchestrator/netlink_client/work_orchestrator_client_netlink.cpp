
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>
#include <sys/mman.h>

#include <labstor/constants/constants.h>
#include <labstor/kernel/client/kernel_client.h>

#include <labmods/work_orchestrator/work_orchestrator.h>
#include "work_orchestrator_client_netlink.h"

int labstor::kernel::netlink::WorkerClient::CreateWorkers(int num_workers, int region_id, size_t region_size, size_t time_slice_us) {
    AUTO_TRACE("labstor::kernel::netlink::WorkerClient::CreateWorkers")
    int code;
    struct labstor_spawn_worker_request rq;
    rq.header_.ns_id_ = WORKER_MODULE_RUNTIME_ID;
    rq.header_.op_ = LABSTOR_SPAWN_WORKERS;
    rq.num_workers_ = num_workers;
    rq.region_id_ = region_id;
    rq.region_size_ = region_size;
    rq.time_slice_us_ = time_slice_us;
    num_workers_ = num_workers;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&code, sizeof(code));
    return region_id;
}

void labstor::kernel::netlink::WorkerClient::AssignQueuePairs(std::vector<labstor_assign_qp_request> &assign_qp_vec) {
    AUTO_TRACE("labstor::kernel::netlink::WorkerClient::AssignQueuePair");
    int code;
    uint32_t size = labstor_assign_qp_request_vec::GetSize(assign_qp_vec.size());
    struct labstor_assign_qp_request_vec *rq_vec = (struct labstor_assign_qp_request_vec*)malloc(size);
    rq_vec->header_.ns_id_ = WORKER_MODULE_RUNTIME_ID;
    rq_vec->header_.op_ = LABSTOR_ASSIGN_QP;
    rq_vec->count_ = assign_qp_vec.size();
    for(int i = 0; i < assign_qp_vec.size(); ++i) {
        rq_vec->rqs_[i].worker_id_ = assign_qp_vec[i].qp_->GetQID().Hash() % num_workers_;
        rq_vec->rqs_[i].qp_ptr_ = assign_qp_vec[i].qp_ptr_;
    }
    kernel_client_->SendMSG(rq_vec, size);
    kernel_client_->RecvMSG(&code, sizeof(code));
    free(rq_vec);
}

void labstor::kernel::netlink::WorkerClient::SetAffinity(int cpu_id) {
    AUTO_TRACE("labstor::kernel::netlink::WorkerClient::SetAffinity")
    struct labstor_set_worker_affinity_request rq;
    int code;
    rq.header_.ns_id_ = WORKER_MODULE_RUNTIME_ID;
    rq.header_.op_ = LABSTOR_SET_WORKER_AFFINITY;
    rq.worker_id_ = worker_id_;
    rq.cpu_id_ = cpu_id;
    kernel_client_->SendMSG(&rq, sizeof(rq));
    kernel_client_->RecvMSG(&code, sizeof(code));
}

void labstor::kernel::netlink::WorkerClient::Start() {}
void labstor::kernel::netlink::WorkerClient::Stop() {}
void labstor::kernel::netlink::WorkerClient::Pause() {}
void labstor::kernel::netlink::WorkerClient::Resume() {}
void labstor::kernel::netlink::WorkerClient::Wait() {}