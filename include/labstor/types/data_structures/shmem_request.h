//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_REQUEST_KERNEL_H
#define LABSTOR_REQUEST_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_qtok.h>

struct labstor_request {
    labstor_req_id_t req_id_;
    uint32_t ns_id_;
    uint16_t op_;
#ifdef __cplusplus
    inline labstor_request() = default;
    inline labstor_request(uint32_t ns_id, uint32_t op) {
        Init(ns_id, op);
    }
    inline labstor_request(labstor_req_id_t req_id, uint32_t ns_id, uint32_t op) {
        Init(req_id, ns_id, op);
    }
    inline void Init(uint32_t ns_id, uint32_t op) {
        ns_id_ = ns_id;
        op_ = op;
    }
    inline void Init(uint32_t req_id, uint32_t ns_id, uint16_t op) {
        req_id_ = req_id;
        ns_id_ = ns_id;
        op_ = op;
    }
    inline uint32_t GetNamespaceID() { return ns_id_; }
    inline uint32_t GetRequestID() { return req_id_; }
    inline uint16_t GetOp() { return op_; }
#endif
};

struct labstor_reply {
    labstor_req_id_t req_id_;
    int32_t code_;
    uint16_t padding_;
#ifdef __cplusplus
    inline labstor_reply() = default;
    inline labstor_reply(int code) {
        code_ = code;
    }
    void Init(labstor_req_id_t req_id, int32_t code) {
        req_id_ = req_id;
        code_ = code;
    }
    inline int GetCode() { return code_; }
    inline void SetCode(int code) { code_ = code; }
#endif
};

struct labstor_poll_single {
    struct labstor_request header_;
    struct labstor_qtok_t poll_qtok_;
    struct labstor_qtok_t reply_qtok_;
#ifdef __cplusplus
    void Init(int req_id, int ns_id, int op, labstor::ipc::qtok_t &poll_qtok, labstor::ipc::qtok_t &reply_qtok) {
        header_.Init(req_id, ns_id, op);
        poll_qtok_ = poll_qtok;
        reply_qtok_ = reply_qtok;
    }
    void Init(int req_id, int ns_id, int op, labstor::ipc::qtok_t poll_qtok, labstor::ipc::qid_t reply_qid, labstor::ipc::req_id_t reply_req_id) {
        header_.Init(req_id, ns_id, op);
        poll_qtok_ = poll_qtok;
        reply_qtok_.Init(reply_qid, reply_req_id);
    }
#endif
};

#ifdef __cplusplus

namespace labstor::ipc {
    typedef labstor_request request;
    typedef labstor_reply reply;
}

#endif

#endif //LABSTOR_REQUEST_KERNEL_H
