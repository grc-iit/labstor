//
// Created by lukemartinlogan on 11/30/21.
//

#ifndef LABSTOR_SHMEM_QTOK_H
#define LABSTOR_SHMEM_QTOK_H

typedef uint64_t labstor_qid_t;

struct labstor_qtok_t {
    labstor_qid_t qid;
    uint32_t req_id;
};

#endif //LABSTOR_SHMEM_QTOK_H
