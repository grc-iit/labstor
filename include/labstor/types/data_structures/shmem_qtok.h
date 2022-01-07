//
// Created by lukemartinlogan on 11/30/21.
//

#ifndef LABSTOR_SHMEM_QTOK_H
#define LABSTOR_SHMEM_QTOK_H

#define LABSTOR_QTOK_INVALID(qtok) (qtok.req_id==-1 || qtok.qid==-1)

typedef uint64_t labstor_qid_t;

typedef uint32_t labstor_req_id_t;
struct labstor_qtok_t {
    labstor_qid_t qid;
    labstor_req_id_t req_id;
#ifdef __cplusplus
    labstor_qtok_t() = default;
    void Init(labstor_qid_t qid_, labstor_req_id_t req_id_) {
        qid = qid_;
        req_id = req_id_;
    }
#endif
};


#ifdef __cplusplus
namespace labstor::ipc {
    typedef uint64_t qid_t;
    typedef labstor_qtok_t qtok_t;
    typedef labstor_req_id_t req_id_t;
}
#endif

#endif //LABSTOR_SHMEM_QTOK_H
