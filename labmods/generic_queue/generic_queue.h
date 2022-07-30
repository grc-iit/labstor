
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

#ifndef LABSTOR_GENERIC_QUEUE_H
#define LABSTOR_GENERIC_QUEUE_H

#include <labstor/types/data_structures/shmem_request.h>
#ifdef __cplusplus
#include <labmods/generic_block/generic_block.h>
#endif

#ifdef __cplusplus
namespace labstor::GenericQueue {

struct io_request : public labstor::ipc::request {
    size_t off_;
    size_t size_;
    void *buf_;
    int hctx_;
    int dev_id_;

    inline void Start(int ns_id, uint16_t op, size_t off, size_t size, void *buf, int hctx) {
        op_ = op;
        ns_id_ = ns_id;
        code_ = 0;
        off_ = off;
        size_ = size;
        buf_ = buf;
        hctx_ = hctx;
    }
};

struct stats_request : public labstor::ipc::request {
    int dev_id_;
    int num_hw_queues_;
    int queue_depth_;
    inline void ClientStart(int ns_id) {
        ns_id_ =  ns_id;
    }
    inline void Start(int ns_id, int dev_id) {
        dev_id_ = dev_id;
    }
    inline void Copy(stats_request *old_rq) {
        num_hw_queues_ = old_rq->num_hw_queues_;
        queue_depth_ = old_rq->queue_depth_;
    }
};

}
#endif

struct labstor_queue_stats_request {
    struct labstor_request header_;
    int dev_id_;
    int num_hw_queues_;
    int queue_depth_;
};

#endif //LABSTOR_GENERIC_QUEUE_H