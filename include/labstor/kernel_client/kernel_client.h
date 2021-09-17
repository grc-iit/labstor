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

#include <labstor/types/module.h>
#include <labstor/types/ipc_pool.h>

#define NETLINK_USER 31

namespace labstor {

class LabStorNetlinkMSG {
private:
    struct nlmsghdr *nlh_;
public:
    LabStorNetlinkMSG() : nlh_(nullptr) {}
    LabStorNetlinkMSG(struct nlmsghdr *nlh) : nlh_(nlh) {}
    ~LabStorNetlinkMSG() { free(nlh_); }
    void *SetNLH(struct nlmsghdr *nlh) { nlh_ = nlh; }
    void *GetData() { NLMSG_DATA(nlh_); }
};

class LabStorKernelClientContext {
private:
    int sockfd_;
public:
    inline bool IsConnected() { return sockfd_ >= 0; }
    bool Connect();
    static inline bool SendMSG(void *serialized_buf, size_t buf_size);
    static inline std::shared_ptr<LabStorNetlinkMSG> RecvMSG(size_t buf_size);
};

}

#endif //LABSTOR_KERNEL_CLIENT_H
