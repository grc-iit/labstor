
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

#ifndef LABSTOR_BLOCK_CLIENT_H
#define LABSTOR_BLOCK_CLIENT_H

#include "labstor/userspace/types/module.h"
#include <labmods/generic_block/generic_block.h>

namespace labstor::GenericBlock {

class Client : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Client(labstor::id module_id) : labstor::Module(module_id) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    virtual labstor::ipc::qtok_t AIO(void *buf, size_t size, size_t off, Ops op) = 0;
    inline labstor::ipc::qtok_t AWrite(void *buf, size_t size, size_t off) {
        return AIO(buf, size, off, Ops::kWrite);
    }
    inline labstor::ipc::qtok_t ARead(void *buf, size_t size, size_t off) {
        return AIO(buf, size, off, Ops::kRead);
    }

    inline int IO(void *buf, size_t size, size_t off, Ops op) {
        labstor::ipc::qtok_t qtok;
        qtok = AIO(buf, size, off, op);
        labstor::GenericBlock::io_request *rq = ipc_manager_->Wait<labstor::GenericBlock::io_request>(qtok);
        int code = rq->GetCode();
        ipc_manager_->FreeRequest<labstor::GenericBlock::io_request>(qtok, rq);
        return code;
    }
    inline int Write(void *buf, size_t size, size_t off) {
        return IO(buf, size, off, Ops::kWrite);
    }
    inline int Read(void *buf, size_t size, size_t off) {
        return IO(buf, size, off, Ops::kRead);
    }
};

}

#endif //LABSTOR_BLOCK_CLIENT_H