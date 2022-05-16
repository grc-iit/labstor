//
// Created by lukemartinlogan on 3/26/22.
//

#ifndef LABSTOR_GENERIC_QUEUE_SERVER_H
#define LABSTOR_GENERIC_QUEUE_SERVER_H

#include <labstor/userspace/server/server.h>
#include <labstor/userspace/types/module.h>
#include <labmods/generic_block/generic_block.h>
#include "labmods/generic_queue/generic_queue.h"

namespace labstor::GenericQueue {

class Server : public labstor::Module {
public:
    Server(std::string module_id) : labstor::Module(module_id) {}
    virtual bool GetStatistics(labstor::queue_pair *qp, stats_request *rq_submit, labstor::credentials *creds) = 0;
};

}

#endif //LABSTOR_GENERIC_QUEUE_SERVER_H
