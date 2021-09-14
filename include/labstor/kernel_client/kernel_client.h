//
// Created by lukemartinlogan on 9/11/21.
//

#ifndef LABSTOR_KERNEL_CLIENT_H
#define LABSTOR_KERNEL_CLIENT_H

#include <pthread.h>
#include <memory>
#include <vector>

#include <labstor/types/package.h>
#include <labstor/types/ipc_pool.h>

#define NETLINK_USER 31

namespace labstor {

struct km_startup_request {
    int num_queues;
    size_t queue_size;
    void *starting_address;
};

class LabStorKernelClientContext {
private:
    int sockfd_;
public:
    inline bool IsConnected() { return sockfd_ >= 0; }
    bool Connect(int num_queues, size_t queue_size);
private:
    void CreateIPC(int num_queues, size_t queue_size);
    static inline struct nlmsghdr *SendStartupMSG(int num_queues, size_t queue_size);
};

}

#endif //LABSTOR_KERNEL_CLIENT_H
