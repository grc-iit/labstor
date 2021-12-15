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
    work_queue_depth = work_queue_.GetDepth();
    LABSTOR_ERROR_HANDLE_START()
    for(uint32_t i = 0; i < work_queue_depth; ++i) {
        if(!work_queue_.Peek(qp, creds, i)) { break; }
        qp_depth = qp->GetDepth();
        for(uint32_t j = 0; j < qp_depth; ++j) {
            if(!qp->Dequeue(rq)) { break; }
            module = namespace_->Get(rq->ns_id_);
            module->ProcessRequest(qp, rq, creds);
        }
    }
    LABSTOR_ERROR_HANDLE_END()
}