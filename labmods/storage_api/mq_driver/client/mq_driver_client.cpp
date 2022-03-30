//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <labmods/registrar/registrar.h>

#include "mq_driver.h"
#include "mq_driver_client.h"
#include "generic_queue/generic_queue.h"

void labstor::MQDriver::Client::Register(const std::string &dev_path, int dev_id) {
    AUTO_TRACE("")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance<register_request>(MQ_DRIVER_MODULE_ID, dev_path, dev_id);
    dev_id_ = dev_id;
    TRACEPOINT(ns_id_)
}

int labstor::MQDriver::Client::GetNamespaceID() {
    auto registrar = labstor::Registrar::Client();
    if(ns_id_ == 0) {
        ns_id_ = registrar.GetNamespaceID(MQ_DRIVER_MODULE_ID);
    }
    return ns_id_;
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