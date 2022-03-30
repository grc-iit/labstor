//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_MQ_DRIVER_H
#define LABSTOR_MQ_DRIVER_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include <labmods/registrar/registrar.h>

#define NO_OP_IOSCHED_MODULE_ID "NO_OP"

namespace labstor::iosched::NoOp {

struct register_request : public labstor::Registrar::register_request {
    labstor::id next_;
    void ConstructModuleStart(
            const std::string &module_id, const std::string &key, const std::string &next_module) {
        labstor::Registrar::register_request::ConstructModuleStart(module_id, key);
        next_.copy(next_module);
    }
};

}

#endif //LABSTOR_MQ_DRIVER_H
