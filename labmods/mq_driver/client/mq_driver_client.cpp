
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
#include <labmods/registrar/registrar.h>

#include "labmods/mq_driver/mq_driver.h"
#include "mq_driver_client.h"
#include "labmods/generic_queue/generic_queue.h"
#include "labmods/blkdev_table/client/blkdev_table_client.h"

void labstor::MQDriver::Client::Register(YAML::Node config) {
    AUTO_TRACE("")
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(MQ_DRIVER_MODULE_ID, config["labmod_uuid"].as<std::string>());
    labstor::BlkdevTable::Client *blkdev_table = LABSTOR_NAMESPACE->LoadClientModule<labstor::BlkdevTable::Client>(BLKDEV_TABLE_MODULE_ID);
    if(!blkdev_table) {
        throw NOT_YET_IMPLEMENTED.format();
    }
    int dev_id = blkdev_table->RegisterBlkdev(config["dev_path"].as<std::string>());
    LABSTOR_REGISTRAR->InitializeInstance<register_request>(ns_id_, dev_id);
    dev_id_ = dev_id;
}

labstor::ipc::qtok_t labstor::MQDriver::Client::AIO(Ops op, void *user_buf, size_t buf_size, size_t sector, int hctx) {
    AUTO_TRACE(buf_size);
    io_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    TRACEPOINT("Submit")
    client_rq = ipc_manager_->AllocRequest<io_request>(qp);
    client_rq->IOClientStart(ns_id_, ipc_manager_->GetPID(), op, user_buf, buf_size, sector, hctx);

    //Enqueue the request
    qp->Enqueue<io_request>(client_rq, qtok);
    return qtok;
}
void labstor::MQDriver::Client::IO(Ops op, void *user_buf, size_t buf_size, size_t sector, int hctx) {
    AUTO_TRACE(dev_id_, buf_size);
    io_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    TRACEPOINT("Submit", "dev_id", dev_id_)
    client_rq = ipc_manager_->AllocRequest<io_request>(qp);
    client_rq->IOClientStart(ns_id_, ipc_manager_->GetPID(), op, user_buf, buf_size, sector, hctx);

    //Complete CLIENT -> SERVER interaction
    qp->Enqueue<io_request>(client_rq, qtok);
    TRACEPOINT("Request_id", client_rq->req_id_);
    client_rq = ipc_manager_->Wait<io_request>(qtok);
    TRACEPOINT("return_code",
               (int)client_rq->op_);

    //Free requests
    ipc_manager_->FreeRequest<io_request>(qtok, client_rq);
}
int labstor::MQDriver::Client::GetNumHWQueues() {
    AUTO_TRACE("");
    labstor::GenericQueue::stats_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    int num_hw_queues = 0;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    TRACEPOINT("Submit", "dev_id", dev_id_)
    client_rq = ipc_manager_->AllocRequest<labstor::GenericQueue::stats_request>(qp);
    client_rq->ClientStart(ns_id_);

    //Complete CLIENT -> SERVER interaction
    qp->Enqueue<labstor::GenericQueue::stats_request>(client_rq, qtok);
    TRACEPOINT("Request_id", client_rq->req_id_);
    client_rq = ipc_manager_->Wait<labstor::GenericQueue::stats_request>(qtok);
    num_hw_queues = client_rq->num_hw_queues_;
    TRACEPOINT("return_code",
               (int)client_rq->op_);

    //Free requests
    ipc_manager_->FreeRequest<labstor::GenericQueue::stats_request>(qtok, client_rq);
    return num_hw_queues;
}

LABSTOR_MODULE_CONSTRUCT(labstor::MQDriver::Client, MQ_DRIVER_MODULE_ID);