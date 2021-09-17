//
// Created by lukemartinlogan on 9/1/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <memory>

#include <labstor/kernel_client/kernel_client.h>

static int sockfd;
static struct sockaddr_nl my_addr = {0};
static struct sockaddr_nl kern_addr = {0};

bool labstor::LabStorKernelClientContext::Connect()
{
    sockfd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_USER);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.nl_family = AF_NETLINK;
    my_addr.nl_pid = getpid();

    memset(&kern_addr, 0, sizeof(kern_addr));
    kern_addr.nl_family = AF_NETLINK;
    kern_addr.nl_pid = 0;

    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    return true;
}

inline bool labstor::LabStorKernelClientContext::SendMSG(void *serialized_buf, size_t buf_size) {
    int num_io_rqs = 0;
    struct nlmsghdr *nlh;
    socklen_t addrlen = sizeof(struct sockaddr_nl);
    int ret;
    void *rq;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(buf_size));
    memset(nlh, 0, NLMSG_SPACE(buf_size));
    nlh->nlmsg_len = NLMSG_SPACE(buf_size);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    rq = NLMSG_DATA(nlh);
    memcpy(rq, serialized_buf, buf_size);

    ret = sendto(sockfd, (void*)nlh, NLMSG_SPACE(buf_size), 0, (struct sockaddr *)&kern_addr, addrlen);
    if(ret < 0) {
        perror("Unable to send message to kernel module\n");
        free(nlh);
        return false;
    }
    free(nlh);
    return true;
}

inline std::shared_ptr<labstor::LabStorNetlinkMSG> labstor::LabStorKernelClientContext::RecvMSG(size_t buf_size) {
    int num_io_rqs = 0;
    struct nlmsghdr *nlh;
    socklen_t addrlen = sizeof(struct sockaddr_nl);
    int ret;
    void *rq;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(buf_size));
    memset(nlh, 0, NLMSG_SPACE(buf_size));
    nlh->nlmsg_len = NLMSG_SPACE(buf_size);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    ret = recvfrom(sockfd, (void*)nlh, NLMSG_SPACE(buf_size), 0, (struct sockaddr *)&kern_addr, &addrlen);
    if(ret < 0) {
        perror("Unable to recv count from kernel module\n");
        free(nlh);
        return NULL;
    }
    rq = NLMSG_DATA(nlh);

    return std::shared_ptr<LabStorNetlinkMSG>(new LabStorNetlinkMSG(nlh));
}