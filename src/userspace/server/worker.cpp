
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