//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_REQUEST_KERNEL_H
#define LABSTOR_REQUEST_KERNEL_H

#include <labstor/types/basics.h>

struct labstor_request {
    uint32_t req_id_;
    uint32_t ns_id_;
    uint16_t op_;
#ifdef __cplusplus
    inline labstor_request() = default;
    inline labstor_request(uint32_t ns_id, uint32_t op) {
        ns_id_ = ns_id;
        op_ = op;
    }
    inline labstor_request(uint32_t req_id, uint32_t ns_id, uint32_t op) {
        req_id_ = req_id;
        ns_id_ = ns_id;
        op_ = op;
    }
    inline uint32_t GetNamespaceID() { return ns_id_; }
    inline uint16_t GetOp() { return op_; }
#endif
};

#ifdef __cplusplus

namespace labstor::ipc {
    typedef labstor_request request;
}

#endif

#endif //LABSTOR_REQUEST_KERNEL_H
