//
// Created by lukemartinlogan on 9/1/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <labstor/kernel_client/kernel_client.h>

static int sockfd;
static struct sockaddr_nl my_addr = {0};
static struct sockaddr_nl kern_addr = {0};

bool labstor::LabStorKernelClientContext::Connect(int num_queues, size_t queue_size)
{
    sockfd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_USER);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.nl_family = AF_NETLINK;
    my_addr.nl_pid = getpid();

    memset(&kern_addr, 0, sizeof(kern_addr));
    kern_addr.nl_family = AF_NETLINK;
    kern_addr.nl_pid = 0;

    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));

    return CreateIPC(num_queues, queue_size);
}

bool labstor::LabStorKernelClientContext::CreateIPC(int num_queues, size_t queue_size)
{
    struct nlmsghdr *nlh;
    struct labstor::km_startup_request *rq;
    int code;

    //Send startup message to labstor kernel server and get response
    nlh = SendStartupMSG(num_queues, queue_size);
    if(nlh == NULL) {
        return false;
    }
    rq = (struct labstor::km_startup_request*)NLMSG_DATA(nlh);
    code = rq->code;
    printf("IPC CODE: %d\n", code);

    free(nlh);
    return true;
}

inline struct nlmsghdr *labstor::LabStorKernelClientContext::SendStartupMSG(int num_queues, size_t queue_size)
{
    int num_io_rqs = 0;
    struct nlmsghdr *nlh;
    struct km_startup_request *rq;
    socklen_t addrlen = sizeof(struct sockaddr_nl);
    int ret;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct km_startup_request)));
    memset(nlh, 0, NLMSG_SPACE(sizeof(struct km_startup_request)));
    nlh->nlmsg_len = NLMSG_SPACE(sizeof(struct km_startup_request));
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    rq = (struct km_startup_request*)NLMSG_DATA(nlh);
    rq->code = 1;
    rq->num_queues = num_queues;
    rq->queue_size = queue_size;

    ret = sendto(sockfd, (void*)nlh, NLMSG_SPACE(sizeof(struct km_startup_request)), 0, (struct sockaddr *)&kern_addr, addrlen);
    if(ret < 0) {
        perror("Unable to send message to kernel module\n");
        free(nlh);
        return NULL;
    }

    ret = recvfrom(sockfd, (void*)nlh, NLMSG_SPACE(sizeof(struct km_startup_request)), 0, (struct sockaddr *)&kern_addr, &addrlen);
    if(ret < 0) {
        perror("Unable to recv count from kernel module\n");
        free(nlh);
        return NULL;
    }

    return nlh;
}