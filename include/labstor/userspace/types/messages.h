
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

#ifndef LABSTOR_MESSAGES_H
#define LABSTOR_MESSAGES_H

#include <labstor/types/data_structures/shmem_qtok.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"

namespace labstor::ipc {

enum {
    LABSTOR_ADMIN_REGISTER_QP
};

struct admin_request {
    int op_;
    admin_request() {}
    admin_request(int op) : op_(op) {}
};

struct admin_reply {
    int code_;
    admin_reply() {}
    admin_reply(int code) : code_(code) {}
};

struct setup_reply : public admin_reply {
    uint32_t region_id_;
    uint32_t region_size_;
    uint32_t request_region_size_;
    uint32_t request_unit_;
    uint32_t queue_region_size_;
    uint32_t queue_depth_;
    uint32_t num_queues_;
    uint32_t namespace_region_id_;
    uint32_t namespace_region_size_;
    uint32_t namespace_max_entries_;
};

struct register_qp_request : public labstor::ipc::admin_request {
    int count_;
    register_qp_request() {}
    register_qp_request(int count) : count_(count), labstor::ipc::admin_request(LABSTOR_ADMIN_REGISTER_QP) {}

    uint32_t GetQueueArrayLength() {
        return count_ * sizeof(labstor::ipc::queue_pair_ptr);
    }
};
typedef admin_reply register_qp_reply;

struct poll_request : public labstor::ipc::request {
    labstor::ipc::qtok_t qtok_;
    labstor::ipc::qtok_t *qtoks_;

    void Init(int ns_id, int op, labstor::ipc::qtok_t qtok) {
        ns_id_ = ns_id;
        op_ = op;
    }

    void Init(int ns_id, int op, labstor::ipc::qtok_t *qtoks) {
        ns_id_ = ns_id;
        op_ = op;
        qtoks_ = qtoks;
    }
};

}

#endif //LABSTOR_MESSAGES_H