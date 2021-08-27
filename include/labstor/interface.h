//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_INTERFACE_H
#define LABSTOR_INTERFACE_H

#include <boost/interprocess/shared_memory_object.hpp>

struct request {
    char *token;
};

void (*process_request_fn)(void *request);

#endif //LABSTOR_INTERFACE_H
