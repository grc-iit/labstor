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
    PerProcessIPC(int sockfd, struct credentials *creds) : sockfd_(sockfd), creds_(*creds) {}

    bool IsConnected() { return sockfd_ >= 0; }
    labstor::ipc::queue_pair& GetQueuePair(int i) { return qps_[i]; }
    int GetNumQueues() { return qps_.size(); }
    int GetSocket() { return sockfd_; }
};

}

#endif //LABSTOR_IPC_POOL_H
