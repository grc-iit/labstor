//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_DUMMY_H
#define LABSTOR_DUMMY_H

#include <labstor/types/data_structures/shmem_request.h>

namespace labstor::test::Dummy {

enum class Ops {
    kRegister,
    kGetValue
};

struct dummy_submit_request : public labstor::ipc::request {
    int value_;
    dummy_submit_request() : labstor::ipc::request() {
    }
};

struct dummy_complete_request : public labstor::ipc::request {
    int num_;

    void Init(int num) {
        num_ = num;
    }
};

}

#endif //LABSTOR_DUMMY_H
