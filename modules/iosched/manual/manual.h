//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_IOSCHED_MANUAL_H
#define LABSTOR_IOSCHED_MANUAL_H

#include <labstor/types/data_stuctures/shmem_request.h>

namespace labstor::iosched {

enum class ManualOps {
    kWriteOp,
    kReadOp,
    kPoll
};

struct write_request : public labstor::ipc::request {
    write_request() {}
};

}

#endif //LABSTOR_IOSCHED_MANUAL_H
