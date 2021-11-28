//
// Created by lukemartinlogan on 9/7/21.
//

#include <labstor/util/debug.h>
#include <labstor/types/module.h>
#include <labstor/userspace_server/worker.h>

void labstor::Server::Worker::DoWork() {
    labstor::ipc::queue_pair qp;
    labstor::ipc::request *rq;
    uint32_t length = work_queue_.GetLength();
    for(uint32_t i = 0; i < length; ++i) {
        if(!work_queue_.Dequeue(qp)) { break; }
        while(qp.sq.Dequeue(rq)) {
            TRACEPOINT("Dequeuing request in worker", rq->ns_id_, rq->op_);
            labstor::Module *module = namespace_->Get(rq->ns_id_);
            module->ProcessRequest(qp, rq, qp.sq.GetCredentials());
        }
        work_queue_.Enqueue(qp);
    }
}