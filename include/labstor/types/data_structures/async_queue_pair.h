//
// Created by lukemartinlogan on 3/3/22.
//

#ifndef LABSTOR_ASYNC_QUEUE_PAIR_H
#define LABSTOR_ASYNC_QUEUE_PAIR_H

#include "queue_pair.h"

namespace labstor {

struct async_request : labstor::ipc::request {
    bool is_complete_;
};

class async_queue_pair : public user_queue_pair {
    inline virtual bool _Enqueue(labstor::ipc::request *rq, labstor::ipc::qtok_t &qtok) = 0;
    inline bool _IsComplete(labstor_req_id_t req_id, labstor::ipc::request **rq) = 0;

    uint32_t GetDepth() { return 0; }
    inline bool _Dequeue(labstor::ipc::request **rq) { return false; }
    inline void _Complete(labstor_req_id_t req_id, labstor::ipc::request *rq) {}
};

}

#endif //LABSTOR_ASYNC_QUEUE_PAIR_H
