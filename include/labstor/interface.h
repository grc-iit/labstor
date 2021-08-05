//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_INTERFACE_H
#define LABSTOR_INTERFACE_H

struct request_queue {
    pid_t pid;
    void *requests;
    int req_size;
};

void (*process_request_fn)(void *request);

#endif //LABSTOR_INTERFACE_H
