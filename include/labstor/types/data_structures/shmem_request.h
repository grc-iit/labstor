//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_REQUEST_KERNEL_H
#define LABSTOR_REQUEST_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#ifdef __cplusplus
#include <cstring>
#endif

struct labstor_request {
    labstor_req_id_t req_id_;
    union {
        uint32_t ns_id_;
        uint32_t code_;
    };
    uint16_t op_;
#ifdef __cplusplus
    inline labstor_request() = default;
    inline void Start(uint32_t req_id, uint32_t ns_id, uint16_t op) {
        req_id_ = req_id;
        ns_id_ = ns_id;
        op_ = op;
    }
    inline void Complete(uint32_t code) {
        SetCode(code);
    }
    inline void Copy(labstor_request *rq) {
        code_ = rq->code_;
    }
    inline uint32_t GetNamespaceID() { return ns_id_; }
    inline uint32_t GetCode() { return code_; }
    inline uint32_t GetRequestID() { return req_id_; }
    inline uint16_t GetOp() { return op_; }

    inline void SetNamespaceID(uint32_t ns_id) {  ns_id_ = ns_id; }
    inline void SetCode(uint32_t code) { code_ = code; }
    inline void SetRequestID(uint32_t req_id) { req_id_ = req_id; }
    inline void SetOp(uint32_t op) { op_ = op; }
#endif
};



#ifdef __cplusplus

namespace labstor::ipc {
    typedef labstor_request request;
}

#endif

#endif //LABSTOR_REQUEST_KERNEL_H
