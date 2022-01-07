//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>

#include "mq_driver.h"
#include "mq_driver_client.h"

void labstor::MQDriver::Client::Register() {
    AUTO_TRACE("")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance(MQ_DRIVER_MODULE_ID, MQ_DRIVER_MODULE_ID);
    TRACEPOINT(ns_id_)
}

void labstor::MQDriver::Client::IO(Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
    AUTO_TRACE(dev_id, buf_size);
    labstor_mq_driver_request *client_rq;
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    TRACEPOINT("Submit", "dev_id", dev_id)
    client_rq = ipc_manager_->AllocRequest<labstor_mq_driver_request>(qp);
    client_rq->Start(ns_id_, ipc_manager_->GetPid(), op, dev_id, user_buf, buf_size, sector, hctx);

    //Complete CLIENT -> SERVER interaction
    qp->Enqueue<labstor_mq_driver_request>(client_rq, qtok);
    TRACEPOINT("Request_id", client_rq->header_.req_id_);
    client_rq = ipc_manager_->Wait<labstor_mq_driver_request>(qtok);
    TRACEPOINT("return_code",
               (int)client_rq->header_.op_);

    //Free requests
    ipc_manager_->FreeRequest<labstor_mq_driver_request>(qtok, client_rq);
}
labstor::ipc::qtok_t labstor::MQDriver::Client::AIO(Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
    AUTO_TRACE(dev_id, buf_size);
    labstor_mq_driver_request *client_rq;
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    TRACEPOINT("Submit", "dev_id", dev_id)
    client_rq = ipc_manager_->AllocRequest<labstor_mq_driver_request>(qp);
    client_rq->Start(ns_id_, ipc_manager_->GetPid(), op, dev_id, user_buf, buf_size, sector, hctx);

    //Enqueue the request
    qp->Enqueue<labstor_mq_driver_request>(client_rq, qtok);
    return qtok;
}

LABSTOR_MODULE_CONSTRUCT(labstor::MQDriver::Client);