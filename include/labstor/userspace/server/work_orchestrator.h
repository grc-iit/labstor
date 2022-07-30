
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

#ifndef LABSTOR_SERVER_WORK_ORCHESTRATOR_H
#define LABSTOR_SERVER_WORK_ORCHESTRATOR_H

#include <sys/sysinfo.h>
#include <memory>
#include <unordered_map>
#include <vector>

#include <labstor/userspace/server/worker.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"

namespace labstor::Server {

class WorkOrchestrator {
private:
    int pid_;
    int n_cpu_;
    pthread_t mapper_;
    std::unordered_map<pid_t, std::vector<std::shared_ptr<labstor::Daemon>>> worker_pool_;
    std::shared_ptr<labstor::Daemon> work_balancer_;
public:
    WorkOrchestrator() {
        pid_ = getpid();
        n_cpu_ = get_nprocs_conf();
    }

    inline int GetPID() { return pid_; }
    inline int GetNumCPU() { return n_cpu_; }
    void CreateWorkers();
    void AssignQueuePair(labstor::ipc::shmem_queue_pair *qp, int worker_id=-1);
};

}

#endif //LABSTOR_SERVER_WORK_ORCHESTRATOR_H