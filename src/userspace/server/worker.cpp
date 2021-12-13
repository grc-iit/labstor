//
// Created by lukemartinlogan on 9/7/21.
//

#include <labstor/userspace/server/server.h>
#include <labstor/constants/debug.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/worker.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/ipc_manager.h>

void labstor::Server::Worker::DoWork() {
    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    labstor::ipc::queue_pair_ptr ptr;
    labstor::ipc::queue_pair qp;
    labstor::ipc::request *rq;
    labstor::credentials *creds;
    labstor::Module *module;
    uint32_t work_queue_depth = work_queue_.GetDepth(), qp_depth;
    void *base;

    TRACEPOINT("Worker started");
    while(1);

    LABSTOR_ERROR_HANDLE_START()
    for(uint32_t i = 0; i < work_queue_depth; ++i) {
        if(!work_queue_.Dequeue(ptr)) { break; }
        base = ipc_manager_->GetRegion(ptr, creds);
        qp.Attach(ptr, base);
        qp_depth = qp.GetDepth();
        for(uint32_t j = 0; j < qp_depth; ++j) {
            if(!qp.Dequeue(rq)) { break; }
            //TRACEPOINT("labstor::Server::Worker::DoWork", rq->ns_id_, rq->op_, rq->req_id_, creds->pid);
            module = namespace_->Get(rq->ns_id_);
            module->ProcessRequest(&qp, rq, creds);
        }
        qp.GetPointer(ptr, base);
        work_queue_.Enqueue(ptr);
    }
    LABSTOR_ERROR_HANDLE_END()
}