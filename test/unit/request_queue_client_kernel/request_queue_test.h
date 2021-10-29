//
// Created by lukemartinlogan on 10/21/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_TEST_H
#define LABSTOR_REQUEST_QUEUE_TEST_H

#include <labstor/types/basics.h>

#define REQUEST_QUEUE_TEST_ID "REQUEST_QUEUE_TEST"

enum {
    REQUEST_QUEUE_TEST_PKG_DEQUEUE
};

struct request_queue_test_request {
    int op;
    int region_id;
};

struct request_queue_test_request_netlink {
    struct labstor_netlink_header header;
    struct request_queue_test_request rq;
};

struct simple_request {
    struct labstor_request request_header;
    int data;
};

#endif //LABSTOR_REQUEST_QUEUE_TEST_H
