//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_REQUEST_H
#define LABSTOR_REQUEST_H

#ifdef __cplusplus

#include <stdint.h>

namespace labstor::ipc {

struct request {
    uint32_t qtok_;
    uint32_t ns_id_;
    uint16_t op_;

    request() = default;
    request(uint32_t ns_id, uint32_t op) : ns_id_(ns_id), op_(op) {}
    request(uint32_t qtok, uint32_t ns_id, uint32_t op) : qtok_(qtok), ns_id_(ns_id), op_(op) {}
    inline uint32_t GetNamespaceID() { return ns_id_; }
    inline uint16_t GetOp() { return op_; }
};

}

#endif

#endif //LABSTOR_REQUEST_H
