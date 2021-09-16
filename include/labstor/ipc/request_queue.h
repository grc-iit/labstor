//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

namespace labstor::ipc {

struct request_queue_header {
    int queue_len_;
};

struct request_queue {
    void *region_;

};

struct queue_pair {
    request_queue submission;
    request_queue completion;
};

typedef int qtoken;

}

#endif //LABSTOR_REQUEST_QUEUE_H
