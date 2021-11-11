//
// Created by lukemartinlogan on 8/27/21.
//

#ifndef LABSTOR_IPC_POOL_H
#define LABSTOR_IPC_POOL_H

#include <labstor/types/basics.h>
#include <labstor/ipc/request_queue.h>

#include <sys/socket.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <atomic>

namespace labstor {

class PerProcessIPC {
private:
    int pid_;
    int sockfd_;
    struct credentials creds_;
    std::vector<labstor::ipc::queue_pair> qps_;
public:
    PerProcessIPC() : sockfd_(-1) {}
    PerProcessIPC(int sockfd) : sockfd_(sockfd) {}
    PerProcessIPC(int sockfd, struct credentials *creds) : sockfd_(sockfd), creds_(*creds) {}

    void CreateQueuePairs(void *region, size_t region_size);
    void AttachQueuePairs();
    labstor::ipc::queue_pair& GetQueuePair(int i) { return qps_[i]; }
};

}

#endif //LABSTOR_IPC_POOL_H
