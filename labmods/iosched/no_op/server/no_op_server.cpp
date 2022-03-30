//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <labmods/registrar/registrar.h>
#include <labmods/storage_api/generic_queue/generic_queue.h>
#include <labmods/storage_api/generic_queue/server/generic_queue_server.h>

#include "no_op_server.h"

bool labstor::iosched::NoOp::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE(request->op_, request->req_id_)
    switch (static_cast<labstor::GenericBlock::Ops>(request->op_)) {
        case labstor::GenericBlock::Ops::kWrite:
        case labstor::GenericBlock::Ops::kRead: {
            return IO(qp, reinterpret_cast<labstor::GenericBlock::io_request*>(request), creds);
        }
    }
}

void labstor::iosched::NoOp::Server::Initialize(labstor::ipc::request *rq) {
    //Need to get next module
    register_request *reg_rq = reinterpret_cast<register_request*>(rq);
    next_module_ = namespace_->Get(reg_rq->next_);
    labstor::GenericQueue::Server *mq_dev = namespace_->Get<labstor::GenericQueue::Server>(next_module_)

    //Need to get number of HW queues (submit req to next module)
}

bool labstor::iosched::NoOp::Server::IO(labstor::queue_pair *qp, labstor::GenericBlock::io_request *client_rq, labstor::credentials *creds) {
    labstor::queue_pair *priv_qp;
    labstor::GenericQueue::io_request *rq;
    int hctx = labstor::ThreadLocal::GetTid() % num_hctx_;

    ipc_manager_->GetNextQueuePair(priv_qp, LABSTOR_QP_PRIVATE | LABSTOR_QP_LOW_LATENCY);
    rq = ipc_manager_->AllocRequest<labstor::GenericQueue::io_request>(priv_qp);
    rq->Start(next_module_, client_rq->op_, client_rq->off_, client_rq->size_, client_rq->buf_, hctx);
}

LABSTOR_MODULE_CONSTRUCT(labstor::iosched::NoOp::Server, NO_OP_IOSCHED_MODULE_ID);