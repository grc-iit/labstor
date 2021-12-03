//
// Created by lukemartinlogan on 9/11/21.
//

#ifndef LABSTOR_KERNEL_CLIENT_H
#define LABSTOR_KERNEL_CLIENT_H

#include <pthread.h>
#include <memory>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <labstor/types/basics.h>
#include <labstor/userspace/types/module.h>

#define NETLINK_USER 31

namespace labstor::Kernel {

class NetlinkClient {
private:
    int sockfd_;
public:
    inline bool IsConnected() { return sockfd_ >= 0; }
    bool Connect();
    static bool SendMSG(void *serialized_buf, size_t buf_size);
    static bool RecvMSG(void *buf, size_t buf_size);
};

}

#endif //LABSTOR_KERNEL_CLIENT_H
