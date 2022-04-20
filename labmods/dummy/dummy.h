//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_DUMMY_H
#define LABSTOR_DUMMY_H

#include <labstor/types/data_structures/shmem_request.h>

#define LABSTOR_DUMMY_MODULE_ID "Dummy"

namespace labstor::test::Dummy {

enum class Ops {
    kInit,
    kGetValue=4
};

struct dummy_request : public labstor::ipc::request {
    int value_;
    dummy_request() {}
    void Start(uint32_t ns_id) {
        ns_id_ = ns_id;
        op_ = static_cast<int>(Ops::kGetValue);
        code_ = 0;
    }
    void Complete(uint32_t value) {
        value_ = value;
    }
};

}

#endif //LABSTOR_DUMMY_H
