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

void LabstorKernelClientContext::Connect(int num_queues, size_t queue_size)
{
    sockfd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_USER);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.nl_family = AF_NETLINK;
    my_addr.nl_pid = getpid();

    memset(&kern_addr, 0, sizeof(kern_addr));
    kern_addr.nl_family = AF_NETLINK;
    kern_addr.nl_pid = 0;

    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));

    CreateIPC(num_queues, queue_size);
}

void LabstorKernelClientContext::CreateIPC(int num_queues, size_t queue_size)
{
    struct nlmsghdr *nlh;
    struct labstor::km_starting_request *rq;
    int code;

    //Send startup message to labstor kernel server and get response
    nlh = send_msg_to_kernel(num_queues, queue_size);
    if(nlh == NULL) {
        return -1;
    }
    rq = (struct labstor::km_starting_request*)NLMSG_DATA(nlh);

    /*code = rq->code;
    if(code < 0) {
        free(nlh);
        return code;
    }
    int *data = (int*)rq->data;
    printf("DATA: %d\n", *data);*/

    free(nlh);
}

static inline struct nlmsghdr *LabstorKernelClientContext::SendStartupMSG(int num_queues, size_t queue_size)
{
    int num_io_rqs = 0;
    struct nlmsghdr *nlh;
    struct km_request *rq;
    socklen_t addrlen = sizeof(struct sockaddr_nl);
    int ret;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct km_request)));
    memset(nlh, 0, NLMSG_SPACE(sizeof(struct km_request)));
    nlh->nlmsg_len = NLMSG_SPACE(sizeof(struct km_request));
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    rq = (struct km_request*)NLMSG_DATA(nlh);

    rq->code = code;

    ret = sendto(sockfd, (void*)nlh, NLMSG_SPACE(sizeof(struct km_request)), 0, (struct sockaddr *)&kern_addr, addrlen);
    if(ret < 0) {
        perror("Unable to send message to kernel module\n");
        free(nlh);
        return NULL;
    }

    ret = recvfrom(sockfd, (void*)nlh, NLMSG_SPACE(sizeof(struct km_request)), 0, (struct sockaddr *)&kern_addr, &addrlen);
    if(ret < 0) {
        perror("Unable to recv count from kernel module\n");
        free(nlh);
        return NULL;
    }

    return nlh;
}