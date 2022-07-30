
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

#ifndef LABSTOR_WORKER_CLIENT_NETLINK_H
#define LABSTOR_WORKER_CLIENT_NETLINK_H

#include <labstor/types/daemon.h>
#include <labstor/kernel/client/macros.h>
#include <labstor/kernel/client/kernel_client.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include "labstor/types/data_structures/c/shmem_work_queue.h"
#include <labmods/work_orchestrator/work_orchestrator.h>

namespace labstor::kernel::netlink {

class WorkerClient : public labstor::Daemon {
private:
    int num_workers_;
    int worker_id_;
    LABSTOR_KERNEL_CLIENT_T kernel_client_;
public:
    WorkerClient() = default;
    WorkerClient(int worker_id) : worker_id_(worker_id) {
        kernel_client_ = LABSTOR_KERNEL_CLIENT;
    }

    int CreateWorkers(int num_workers, int region_id, size_t region_size, size_t time_slice_us);
    void AssignQueuePairs(std::vector<struct labstor_assign_qp_request> &assign_qp_vec);

    void Start();
    void Pause();
    void Resume();
    void Wait();
    void Stop();
    void SetAffinity(int cpu_id);
};

}

#endif //LABSTOR_WORKER_CLIENT_NETLINK_H