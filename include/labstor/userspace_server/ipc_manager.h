//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SERVER_IPCMANAGER_H
#define LABSTOR_SERVER_IPCMANAGER_H

#include <vector>
#include <labstor/types/messages.h>
#include <labstor/types/socket.h>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/allocator.h>
#include <labstor/types/data_structures/shmem_request_queue.h>
#include <labstor/userspace_server/macros.h>
#include <mutex>
#include <future>

#define QP_SHMEM 1
#define QP_STREAM 2
#define QP_ORDERED 4
#define QP_SYNCHRONOUS 8

namespace labstor::Server {

struct PerProcessIPC {
    UnixSocket clisock_;
    labstor::credentials creds_;
    void *shmem_region_;
    labstor::GenericAllocator *alloc_;
    std::vector<labstor::queue_pair> qps_;

    PerProcessIPC() {}
    PerProcessIPC(int fd, labstor::credentials creds) : clisock_(fd), creds_(creds) {}
    PerProcessIPC(const PerProcessIPC &old_ipc) {
        clisock_ = old_ipc.clisock_;
        creds_ = old_ipc.creds_;
        shmem_region_ = old_ipc.shmem_region_;
        alloc_ = old_ipc.alloc_;
        qps_ = old_ipc.qps_;
    }
    UnixSocket& GetSocket() { return clisock_; };
};

class IPCManager {
private:
    int server_fd_;
    void *internal_mem_;
    std::mutex register_lock_;
    std::unordered_map<int, PerProcessIPC> pid_to_ipc_;
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
public:
    IPCManager() {
        labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    }
    ~IPCManager() {}

    inline void SetServerFd(int fd) { server_fd_ = fd; }
    inline int GetServerFd() { return server_fd_; }

    void RegisterClient(int client_fd, labstor::credentials &creds);

    void *Allocate(int size);
    void Free(void *ptr);

    void RegisterQP(PerProcessIPC client_ipc, admin_request header);

    labstor::queue_pair& GetQP(size_t qid);
    labstor::queue_pair& GetQP(int flags, int tid);
    labstor::queue_pair& GetQP(int flags, int module_id, int key, int tid);

    void PauseQueues();
    void ResumeQueues();

    std::unordered_map<int, PerProcessIPC>& GetIPCTable() {
        return pid_to_ipc_;
    }
};

}
#endif //LABSTOR_SERVER_IPCMANAGER_H
