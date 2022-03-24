//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_MQ_DRIVER_H
#define LABSTOR_MQ_DRIVER_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
//#include <labstor/types/data_structures/shmem_poll.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"

#define NO_OP_IOSCHED_MODULE_ID "NO_OP"

namespace labstor::iosched::NoOp {
enum class Ops {
    kRegister
};
}
#endif //LABSTOR_MQ_DRIVER_H
