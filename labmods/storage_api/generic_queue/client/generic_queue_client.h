//
// Created by lukemartinlogan on 3/26/22.
//

#ifndef LABSTOR_GENERIC_QUEUE_CLIENT_H
#define LABSTOR_GENERIC_QUEUE_CLIENT_H

#include <labstor/userspace/types/module.h>
#include <labmods/storage_api/generic_block/generic_block.h>

namespace labstor::GenericQueue {

class Client : public labstor::Module {
public:
    Client(labstor::id module_id) : labstor::Module(module_id) {}
};

}

#endif //LABSTOR_GENERIC_QUEUE_CLIENT_H
