//
// Created by lukemartinlogan on 9/1/21.
//

#ifndef TEST_ADDRS_H
#define TEST_ADDRS_H

#define NETLINK_USER 31

struct km_request {
    int code;
    void *data;
    void *fun;
};

#endif //TEST_ADDRS_H
