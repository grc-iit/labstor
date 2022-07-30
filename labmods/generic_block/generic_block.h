
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

#ifndef LABSTOR_BLOCK_H
#define LABSTOR_BLOCK_H

#include "labstor/types/data_structures/shmem_request.h"

namespace labstor::GenericBlock {

enum class Ops {
    kInit,
    kRead,
    kWrite
};

struct io_request : public labstor::ipc::request {
    size_t off_;
    size_t size_;
    void *buf_;

    inline void Start(int ns_id, Ops op, size_t off, size_t size, void *buf) {
        op_ = static_cast<int>(op);
        ns_id_ = ns_id;
        code_ = 0;
        off_ = off;
        size_ = size;
        buf_ = buf;
    }

    inline void Start(int ns_id, Ops op, size_t size, void *buf) {
            op_ = static_cast<int>(op);
            ns_id_ = ns_id;
            code_ = 0;
            off_ = -1;
            size_ = size;
            buf_ = buf;
    }
};

}

#endif //LABSTOR_BLOCK_H