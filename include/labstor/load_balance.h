//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_LOAD_BALANCE_H
#define LABSTOR_LOAD_BALANCE_H

#include <labstor/interface.h>

struct IOWorker {

};

void (*poll_requests_fn)(request_queue *submission_queues);

#endif //LABSTOR_LOAD_BALANCE_H
