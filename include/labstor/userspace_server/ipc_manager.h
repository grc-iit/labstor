//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SERVER_IPCMANAGER_H
#define LABSTOR_SERVER_IPCMANAGER_H

#include <vector>
#include <labstor/types/allocator/allocator.h>
#include <labstor/types/basics.h>
#include <labstor/ipc/request_queue.h>
#include <mutex>

#define QP_SHMEM 1
#define QP_STREAM 2
#define QP_ORDERED 4
#define QP_SYNCHRONOUS 8

namespace labstor::Server {

struct PerProcessIPC {
    int clientfd_;
    void *shmem_region;
    labstor::GenericAllocator *alloc_;
    std::vector<struct queue_pair> qps_;
};

class IPCManager {
private:
    pthread_t thread_;
    int server_fd_;
    void *internal_mem_;
    std::mutex register_lock_;
    std::unordered_map<pid_t, PerProcessIPC> qps_;
public:
    void* WreapProcesses(void*);
    inline void SetServerFd(int fd) { server_fd_ = fd; }
    inline int GetServerFd() { return server_fd_; }

    void RegisterClient(int client_fd, labstor::credentials *creds, int num_queues);
    void *Allocate(int size);
    void Free(void *ptr);

    labstor::ipc::queue_pair& GetQP(size_t qid);
    labstor::ipc::queue_pair& GetQP(int flags, int tid);
    labstor::ipc::queue_pair& GetQP(int flags, int module_id, int key, int tid);

    void PauseQueues();
    void ResumeQueues();
};

}
#endif //LABSTOR_SERVER_IPCMANAGER_H
