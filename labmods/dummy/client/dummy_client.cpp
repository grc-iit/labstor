
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

#include "dummy.h"
#include "dummy_client.h"

void labstor::test::Dummy::Client::Register(YAML::Node config) {
    AUTO_TRACE("")
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(LABSTOR_DUMMY_MODULE_ID, LABSTOR_DUMMY_MODULE_ID);
}

void labstor::test::Dummy::Client::GetValue() {
    AUTO_TRACE(ns_id_)
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    dummy_request *rq;
    labstor::HighResCpuTimer t;
    std::vector<labstor::ipc::qtok_t> qtoks;

    ipc_manager_->GetQueuePair(qp, 0);
    for(int i = 0; i < 100; ++i) {
        rq = ipc_manager_->AllocRequest<dummy_request>(qp);
        rq->Start(ns_id_);
        TRACEPOINT("START", t.GetUsFromEpoch());
        qp->Enqueue(rq, qtok);
        qtoks.emplace_back(qtok);
    }

    for(int i = 0; i < 100; ++i) {
        rq = ipc_manager_->Wait<dummy_request>(qtoks[i]);
        TRACEPOINT("END", t.GetUsFromEpoch());
        ipc_manager_->FreeRequest(qtok, rq);
    }
}

LABSTOR_MODULE_CONSTRUCT(labstor::test::Dummy::Client, LABSTOR_DUMMY_MODULE_ID)