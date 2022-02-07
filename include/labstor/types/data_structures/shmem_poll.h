//
// Created by lukemartinlogan on 12/28/21.
//

#ifndef LABSTOR_SHMEM_POLL_H
#define LABSTOR_SHMEM_POLL_H

#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>

#ifdef __cplusplus

namespace labstor::ipc {

template<typename T>
struct poll_request_single : labstor_request {
    struct labstor_qtok_t poll_qtok_; //The request being polled
    struct labstor_qtok_t reply_qtok_; //The request waiting for the qtok
    T *reply_rq_; //The request that will be used to hold the reply
    void Init(labstor::ipc::queue_pair *qp, T *reply_rq, labstor::ipc::qtok_t &poll_qtok, int op) {
        ns_id_ = reply_rq->header_.ns_id_;
        op_ = op;
        poll_qtok_ = poll_qtok;
        reply_qtok_.qid_ = qp->GetQid();
        reply_qtok_.req_id_ = reply_rq->header_.req_id_;
        reply_rq_ = reply_rq;
    }
};
}

#endif

#endif //LABSTOR_SHMEM_POLL_H
