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
    LABSTOR_ERROR_HANDLE_TRY {
        for (uint32_t i = 0; i < work_queue_depth; ++i) {
            if (!work_queue_.Peek(qp_struct, creds, i)) { break; }
            ipc_manager_->GetQueuePair(qp, qp_struct->GetQID());
            qp_depth = qp->GetDepth();
            for (uint32_t j = 0; j < qp_depth; ++j) {
                if (!qp->Peek(rq, 0)) { break; }
                module = namespace_->GetModule(rq->GetNamespaceID());
                if (!module) {
                    rq->SetCode(-1);
                    qp->Complete(rq);
                    TRACEPOINT("Could not find module in namespace", rq->GetNamespaceID())
                    continue;
                }
                bool is_complete = module->ProcessRequest(qp, rq, creds);
                if(is_complete) {
                    qp->Dequeue(rq);
                } else {
                    break;
                }
            }
        }
    }
    LABSTOR_ERROR_HANDLE_CATCH {
        printf("In worker\n");
        LABSTOR_ERROR_PTR->print();
    };
}