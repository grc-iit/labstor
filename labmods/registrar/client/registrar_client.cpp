
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

#include "registrar.h"
#include "registrar_client.h"

uint32_t labstor::Registrar::Client::GetNamespaceID(std::string key) {
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::namespace_id_request *rq;
    uint32_t ns_id;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<namespace_id_request>(qp);
    rq->GetNamespaceIDStart(key);
    qp->Enqueue(rq, qtok);
    rq = ipc_manager_->Wait<namespace_id_request>(qtok);
    ns_id = rq->GetNamespaceID();
    ipc_manager_->FreeRequest(qtok, rq);
    return ns_id;
}

int labstor::Registrar::Client::MountLabStack(std::string key, std::string yaml_path) {
    return 0;
}

int labstor::Registrar::Client::UnmountLabStack(std::string key, std::string yaml_path) {
    return 0;
}

int labstor::Registrar::Client::PushUpgrade(std::string yaml_path) {
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::upgrade_request *rq;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<upgrade_request>(qp);
    rq->PushUpgradeStart(yaml_path);
    qp->Enqueue(rq, qtok);
    rq = ipc_manager_->Wait<upgrade_request>(qtok);
    int rc = rq->GetCode();
    ipc_manager_->FreeRequest(qtok, rq);
    return rc;
}

std::string labstor::Registrar::Client::GetModulePath(int ns_id) {
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::module_path_request *rq;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<module_path_request>(qp);
    rq->GetModulePathStart(ns_id);
    qp->Enqueue(rq, qtok);
    rq = ipc_manager_->Wait<module_path_request>(qtok);
    std::string path = rq->GetModulePath();
    ipc_manager_->FreeRequest(qtok, rq);
    return std::move(path);
}

void labstor::Registrar::Client::TerminateServer() {
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    labstor::Registrar::terminate_request *rq;

    ipc_manager_->GetQueuePair(qp, 0);
    rq = ipc_manager_->AllocRequest<terminate_request>(qp);
    rq->TerminateStart();
    qp->Enqueue(rq, qtok);
    rq = ipc_manager_->Wait<terminate_request>(qtok);
    ipc_manager_->FreeRequest(qtok, rq);
}

LABSTOR_MODULE_CONSTRUCT(labstor::Registrar::Client, LABSTOR_REGISTRAR_MODULE_ID)