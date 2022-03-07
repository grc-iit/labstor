//
// Created by lukemartinlogan on 11/30/21.
//

#ifndef LABSTOR_SHMEM_QTOK_H
#define LABSTOR_SHMEM_QTOK_H

#ifdef __cplusplus
#include <labstor/userspace/util/serializeable.h>
#endif

#define LABSTOR_QTOK_INVALID(qtok) (qtok.req_id==-1 || qtok.qid==-1)

typedef uint8_t labstor_qid_flags_t;
typedef uint8_t labstor_qid_type_t;

typedef struct labstor_qid_t {
    uint8_t flags_;
    uint8_t type_;
    uint16_t cnt_;
    uint32_t pid_;
#ifdef __cplusplus
    labstor_qid_t() = default;
    labstor_qid_t(uint32_t qid) : flags_(0), cnt_(qid), pid_(0) {}
    uint64_t Hash() {
        uint64_t num = 0;
        num += flags_;
        num <<= 8;
        num += type_;
        num <<= 8;
        num += pid_ % (1<<16);
        num <<= 16;
        num += cnt_;
        return num;
    }
    bool operator ==(const labstor_qid_t &qid) {
        return (qid.flags_ == flags_) && (qid.pid_ == pid_) && (qid.cnt_ == cnt_);
    }
#endif
} labstor_qid_t;

typedef uint32_t labstor_req_id_t;
struct labstor_qtok_t {
    labstor_qid_t qid_;
    labstor_req_id_t req_id_;
#ifdef __cplusplus
    labstor_qtok_t() = default;
    void Init(labstor_qid_t qid_, labstor_req_id_t req_id_) {
        qid_ = qid_;
        req_id_ = req_id_;
    }
#endif
};


#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_qid_t qid_t;
    typedef labstor_qtok_t qtok_t;
    typedef labstor_req_id_t req_id_t;
}
#endif

#endif //LABSTOR_SHMEM_QTOK_H
