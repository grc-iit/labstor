//
// Created by lukemartinlogan on 9/1/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "test_addrs.h"

static int sockfd;
static struct sockaddr_nl my_addr = {0};
static struct sockaddr_nl kern_addr = {0};

static inline struct nlmsghdr *send_msg_to_kernel(int code)
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

void init_kernel_connection(void)
{
    sockfd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_USER);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.nl_family = AF_NETLINK;
    my_addr.nl_pid = getpid();

    memset(&kern_addr, 0, sizeof(kern_addr));
    kern_addr.nl_family = AF_NETLINK;
    kern_addr.nl_pid = 0;

    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
}

int get_kernel_addrs(void)
{
    struct nlmsghdr *nlh;
    struct km_request *rq;
    int code;

    nlh = send_msg_to_kernel(1);
    if(nlh == NULL) {
        return -1;
    }
    rq = (struct km_request*)NLMSG_DATA(nlh);
    code = rq->code;
    if(code < 0) {
        free(nlh);
        return code;
    }

    printf("DATA: %p, FUNCTION: %p\n", rq->data, rq->fun);
    int *data = (int*)rq->data;
    printf("DATA: %d\n", *data);

    free(nlh);
    return code;
}

int main(void) {
    init_kernel_connection();
    get_kernel_addrs();
}