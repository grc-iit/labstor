
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

#include <labstor/constants/debug.h>
#include "spdk_client.h"

void labstor::SPDK::Client::Register(YAML::Node config) {
    std::string traddr = config["traddr"].as<std::string>();
    int nvme_ns_id = config["nvme_ns_id"].as<int>();

    context_.Init();
    context_.Probe();
    context_.SelectDevice(traddr, nvme_ns_id);
    spdk_queue_type_id_ = ipc_manager_->RegisterQueuePairType("SPDK", LABSTOR_QP_PRIVATE | LABSTOR_QP_SHMEM);

    //Create the SPDK queue for housing requests
    TRACEPOINT("Reserving queues", context_.GetNumQueuePairs(), spdk_queue_type_id_)
    ipc_manager_->ReserveQueues(spdk_queue_type_id_, LABSTOR_QP_PRIVATE, context_.GetNumQueuePairs());
    ipc_manager_->ReserveQueues(spdk_queue_type_id_, LABSTOR_QP_SHMEM, context_.GetNumQueuePairs());

    TRACEPOINT("Defining queues")
    for(int i = 0; i < context_.GetNumQueuePairs(); ++i) {
        //Create private queue
        labstor::ipc::shmem_queue_pair *priv_qp = new labstor::ipc::shmem_queue_pair();
        labstor::ipc::qid_t qid = labstor::queue_pair::GetQID(
                spdk_queue_type_id_,
                LABSTOR_QP_PRIVATE,
                i,
                context_.GetNumQueuePairs(),
                ipc_manager_->GetPID());
        int queue_size = labstor::ipc::request_queue::GetSize(context_.GetMaxQueueDepth());
        void *sq_region = ipc_manager_->AllocPrivateQueue(queue_size);
        void *cq_region = ipc_manager_->AllocPrivateQueue(queue_size);
        priv_qp->Init(qid, ipc_manager_->GetRegion(LABSTOR_QP_PRIVATE), sq_region, queue_size, cq_region, queue_size);
        ipc_manager_->RegisterQueuePair(priv_qp);
        TRACEPOINT("Registered Private Queue")

        //Create SPDK queue
        labstor::ipc::qid_t spdk_qid = labstor::queue_pair::GetQID(
                spdk_queue_type_id_,
                LABSTOR_QP_SHMEM,
                i,
                context_.GetNumQueuePairs(),
                ipc_manager_->GetPID());
        labstor::SPDK::queue_pair *spdk_qp = new labstor::SPDK::queue_pair(priv_qp, spdk_qid, context_.GetDevice());
        ipc_manager_->RegisterQueuePair(spdk_qp);
        TRACEPOINT("Registered SHMEM Queue")
    }
}

void* labstor::SPDK::Client::Alloc(size_t size) {
    return context_.Alloc(size);
}

void labstor::SPDK::Client::Free(void *mem) {
    context_.Free(mem);
}

labstor::ipc::qtok_t labstor::SPDK::Client::AIO(Ops op, void *user_buf, size_t buf_size, size_t sector) {
    labstor::queue_pair *qp;
    labstor::SPDK::queue_pair *spdk_qp;
    labstor::SPDK::io_request *rq;
    labstor::ipc::qtok_t qtok;

    ipc_manager_->GetQueuePair(qp, spdk_queue_type_id_, LABSTOR_QP_SHMEM);
    spdk_qp = static_cast<labstor::SPDK::queue_pair*>(qp);
    rq = ipc_manager_->AllocRequest<labstor::SPDK::io_request>(qp);
    rq->Init(op, user_buf, buf_size, sector);
    spdk_qp->Enqueue(rq, qtok);

    return qtok;
}

LABSTOR_MODULE_CONSTRUCT(labstor::SPDK::Client, SPDK_MODULE_ID);