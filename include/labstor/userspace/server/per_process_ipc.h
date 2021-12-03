//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_PER_PROCESS_IPC_H
#define LABSTOR_PER_PROCESS_IPC_H

namespace labstor::Server {
struct PerProcessIPC {
    UnixSocket clisock_;
    labstor::credentials creds_;
    int region_id_;
    void *shmem_region_;
    labstor::GenericAllocator *alloc_;
    int num_stream_qps_;

    PerProcessIPC() {}

    PerProcessIPC(int fd, labstor::credentials creds) : clisock_(fd), creds_(creds) {}

    PerProcessIPC(const PerProcessIPC &old_ipc) {
        clisock_ = old_ipc.clisock_;
        creds_ = old_ipc.creds_;
        shmem_region_ = old_ipc.shmem_region_;
        alloc_ = old_ipc.alloc_;
    }

    inline UnixSocket &GetSocket() { return clisock_; };

    inline void *GetRegion() { return shmem_region_; }
};
}

#endif //LABSTOR_PER_PROCESS_IPC_H
