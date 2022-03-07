//
// Created by lukemartinlogan on 12/3/21.
//

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
