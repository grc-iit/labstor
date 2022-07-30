
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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
    inline bool _Peek(labstor::ipc::request **rq, int i) { return false; }
    inline bool _Dequeue(labstor::ipc::request **rq) { return false; }
    inline void _Complete(labstor_req_id_t req_id, labstor::ipc::request *rq) {}
};

}

#endif //LABSTOR_ASYNC_QUEUE_PAIR_H