
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

#include "blkdev_table_server.h"
#include <labstor/constants/constants.h>

bool labstor::BlkdevTable::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE(request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {
        case Ops::kRegisterBdev: {
            return RegisterBlkdev(qp, reinterpret_cast<blkdev_table_register_request*>(request), creds);
        }
    }
    return false;
}

bool labstor::BlkdevTable::Server::RegisterBlkdev(labstor::queue_pair *qp, blkdev_table_register_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE("qp_ptr", (size_t)qp, "qp_id", qp->GetQID().Hash(), "path", client_rq->path_);
    blkdev_table_register_request *kern_rq;
    labstor::queue_pair *kern_qp;

    //Get KERNEL & PRIVATE QP
    switch(client_rq->GetCode()) {
        case 0: {
            ipc_manager_->GetQueuePairByPid(kern_qp,
                                            LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE |
                                            LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                                            KERNEL_PID);
            //Create SERVER -> KERNEL message
            kern_rq = ipc_manager_->AllocRequest<blkdev_table_register_request>(kern_qp);
            kern_rq->ServerStart(BLKDEV_TABLE_RUNTIME_ID, client_rq);
            if (!dev_ids_.Dequeue(kern_rq->dev_id_)) {
                //TODO; reply error to user and free
            }
            TRACEPOINT("KERN SUBMIT", kern_rq->path_, "dev_id", kern_rq->dev_id_);
            kern_qp->Enqueue<blkdev_table_register_request>(kern_rq, client_rq->kern_qtok_);
            client_rq->SetCode(1);
            return false;
        }
        case 1: {
            ipc_manager_->GetQueuePair(kern_qp, client_rq->kern_qtok_);
            if(!kern_qp->IsComplete<blkdev_table_register_request>(client_rq->kern_qtok_, kern_rq)) {
                return false;
            }
            client_rq->Copy(kern_rq);
            qp->Complete<blkdev_table_register_request>(client_rq);
            ipc_manager_->FreeRequest<blkdev_table_register_request>(kern_qp, kern_rq);
            return true;
        }
    }
    return true;
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Server, BLKDEV_TABLE_MODULE_ID)