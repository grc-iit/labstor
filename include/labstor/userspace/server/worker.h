
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

#ifndef LABSTOR_SERVER_WORKER_H
#define LABSTOR_SERVER_WORKER_H

#ifdef __cplusplus

#include <thread>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/namespace.h>
#include <labstor/types/daemon.h>
#include "labstor/types/data_structures/c/shmem_work_queue_secure.h"

namespace labstor::Server {

class Worker : public DaemonWorker {
private:
    LABSTOR_NAMESPACE_T namespace_;
    void *region_;
    uint32_t id_;
    labstor::ipc::work_queue_secure work_queue_;

    labstor_queue_pair *qp_struct;
    labstor::queue_pair *qp;
    labstor::ipc::request *rq;
    labstor::credentials *creds;
    labstor::Module *module;
    uint32_t work_queue_depth, qp_depth;
    labstor::HighResCpuTimer t;
public:
    Worker(uint32_t depth, uint32_t id) {
        namespace_ = LABSTOR_NAMESPACE;
        id_ = id;
        uint32_t region_size = labstor::ipc::work_queue_secure::GetSize(depth);
        region_ = malloc(region_size);
        work_queue_.Init(region_, region_size, depth);
    }
    void AssignQP(labstor_queue_pair *qp, labstor::credentials *creds) {
        if(!work_queue_.Enqueue(qp, creds)) {
            throw FAILED_TO_ASSIGN_QUEUE.format(qp->GetQID().pid_, id_);
        }
    }
    uint32_t GetQueueDepth() {
        return work_queue_.GetDepth();
    }
    void DoWork();
};

}

#endif

#endif //LABSTOR_SERVER_WORKER_H