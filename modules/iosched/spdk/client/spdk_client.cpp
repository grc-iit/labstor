//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include "spdk_client.h"

void labstor::iosched::SPDK::Client::Init(const std::string &traddr, int nvme_ns_id) {
    context_.Init();
    context_.Probe();
    context_.SelectDevice(traddr, nvme_ns_id);
    context_.CreateQueuePairs(0);
}

void* labstor::iosched::SPDK::Client::Alloc(size_t size) {
    return context_->Alloc(size);
}

void labstor::iosched::SPDK::Client::Free(void *mem) {
    context_->Free(mem);
}

labstor::ipc::qtok_t labstor::iosched::SPDK::Client::AIO(Ops op, void *user_buf, size_t buf_size, size_t sector) {
    labstor::ipc::queue_pair *qp;
    spdk_poll_request *rq;
    labstor::ipc::qtok_t qtok;
    ipc_manager_->GetQueuePair(qp, LABSTOR_QP_PRIVATE | LABSTOR_QP_LOW_LATENCY);
    rq = ipc_manager_->AllocRequest<spdk_poll_request>(qp);
    rq->qp_ = qp;
    qp->Enqueue(rq, qtok);
    qp->Dequeue(rq);
}

void labstor::iosched::SPDK::Client::IOComplete(void *arg, const struct spdk_nvme_cpl *completion) {
    spdk_poll_request *rq = reinterpret_cast<spdk_poll_request*>(arg);
    rq->qp_->Complete(rq);
}

LABSTOR_MODULE_CONSTRUCT(labstor::iosched::SPDK::Client, SPDK_MODULE_ID);