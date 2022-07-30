
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

#ifndef LABSTOR_PER_PROCESS_IPC_H
#define LABSTOR_PER_PROCESS_IPC_H

#include <labstor/types/allocator/allocator.h>
#include <labstor/types/allocator/segment_allocator.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/types/queue_pool.h>
#include <vector>
#include "labstor/userspace/types/memory_manager.h"

namespace labstor::Server {
struct PerProcessIPC : public QueuePool, public MemoryManager {
    UnixSocket clisock_;
    labstor::credentials creds_;
    int region_id_;

    PerProcessIPC(int pid) {
        creds_.pid_ = pid;
    }

    PerProcessIPC(int fd, labstor::credentials creds) : clisock_(fd), creds_(creds) {}

    inline UnixSocket &GetSocket() { return clisock_; };

    inline void *GetRegion() { return MemoryManager::GetRegion(LABSTOR_QP_SHMEM); }

    inline int GetPID() { return creds_.pid_; }
};
}

#endif //LABSTOR_PER_PROCESS_IPC_H