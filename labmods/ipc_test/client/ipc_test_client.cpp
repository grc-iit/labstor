
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


#include "ipc_test.h"
#include "ipc_test_client.h"

void labstor::IPCTest::Client::Register(YAML::Node config) {
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(IPC_TEST_MODULE_ID, IPC_TEST_MODULE_ID);
}

int labstor::IPCTest::Client::Start(int batch_size) {
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtoks[batch_size];
    ipc_test_request *client_rq;
    int dev_id;

    ipc_manager_->GetQueuePair(qp, 0);
    //printf("[tid=%d] QID: %lu\n", labstor::ThreadLocal::GetTid(), qp->GetQID().Hash());
    for(int i = 0; i < batch_size; ++i) {
        client_rq = ipc_manager_->AllocRequest<ipc_test_request>(qp);
        TRACEPOINT("Allocated request")
        client_rq->IPCClientStart(ns_id_, 24);
        qp->Enqueue<ipc_test_request>(client_rq, qtoks[i]);
        TRACEPOINT("Enqueued request")
    }

    int ret = ipc_manager_->Wait<ipc_test_request>(qtoks, batch_size);
    TRACEPOINT("Finished wait")
    if (ret != LABSTOR_REQUEST_SUCCESS) {
        printf("IPC test failed: return code %d\n", ret);
        exit(1);
    }
    TRACEPOINT("Finished")
    return dev_id;
}
LABSTOR_MODULE_CONSTRUCT(labstor::IPCTest::Client, IPC_TEST_MODULE_ID)