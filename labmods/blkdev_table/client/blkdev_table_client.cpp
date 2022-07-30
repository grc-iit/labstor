
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

#include <labstor/constants/debug.h>
#include <labmods/registrar/registrar.h>

#include "blkdev_table.h"
#include "blkdev_table_client.h"

void labstor::BlkdevTable::Client::Register(YAML::Node config) {
    AUTO_TRACE("")
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(BLKDEV_TABLE_MODULE_ID, BLKDEV_TABLE_MODULE_ID);
    TRACEPOINT(ns_id_)
}

int labstor::BlkdevTable::Client::RegisterBlkdev(std::string path) {
    AUTO_TRACE(ns_id_)
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    blkdev_table_register_request *rq;
    int dev_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<blkdev_table_register_request>(qp,
         blkdev_table_register_request::GetSize(path.size()));
    rq->ClientStart(ns_id_, path.c_str(), path.size(), -1);

    TRACEPOINT("path", rq->path_, "qp_id", qp->GetQID().Hash());
    qp->Enqueue<blkdev_table_register_request>(rq, qtok);
    rq = ipc_manager_->Wait<blkdev_table_register_request>(qtok);
    dev_id = rq->GetDeviceID();
    TRACEPOINT("Complete",
               "return_code", rq->GetCode(),
               "device_id", rq->GetDeviceID());
    ipc_manager_->FreeRequest<blkdev_table_register_request>(qtok, rq);
    return dev_id;
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlkdevTable::Client, BLKDEV_TABLE_MODULE_ID)