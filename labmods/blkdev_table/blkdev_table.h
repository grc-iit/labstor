
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

#ifndef LABSTOR_BLKDEV_TABLE_H
#define LABSTOR_BLKDEV_TABLE_H

#define BLKDEV_TABLE_MODULE_ID "BlkdevTable"

#define MAX_MOUNTED_BDEVS 64
#define BDEV_ACCESS_FLAGS FMODE_READ | FMODE_WRITE | FMODE_PREAD | FMODE_PWRITE //| FMODE_EXCL
#define MAX_PAGES_PER_GET 128

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
//#include <labstor/types/data_structures/shmem_poll.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include <labstor/types/data_structures/shmem_qtok.h>

enum BlkdevTableOps {
    LABSTOR_BLKDEV_TABLE_REGISTER,
    LABSTOR_BLKDEV_TABLE_UNREGISTER,
    LABSTOR_BLKDEV_TABLE_REGISTER_BDEV,
    LABSTOR_BLKDEV_TABLE_UNREGISTER_BDEV,
};

#ifdef __cplusplus
namespace labstor::BlkdevTable {
enum class Ops {
    kRegister,
    kUnregister,
    kRegisterBdev,
    kUnregisterBdev,
    kGetBdevID
};

struct blkdev_table_register_request : public labstor::ipc::request {
    struct labstor_qtok_t kern_qtok_;
    int pathlen_;
    uint32_t dev_id_;
    char path_[];
    static inline uint32_t GetSize(uint32_t pathlen) {
        return sizeof(blkdev_table_register_request) + pathlen + 1;
    }
    inline void ClientStart(int ns_id, const char *path, int pathlen, int devid) {
        op_ = static_cast<int>(labstor::BlkdevTable::Ops::kRegisterBdev);
        ns_id_ = ns_id;
        code_ = 0;
        pathlen_ = pathlen;
        memcpy(path_, path, pathlen);
        path_[pathlen_] = 0;
    }
    inline void ServerStart(int ns_id, blkdev_table_register_request *rq) {
        ns_id_ = ns_id;
        op_ = rq->op_;
        pathlen_ = rq->pathlen_;
        memcpy(path_, rq->path_, pathlen_);
        path_[pathlen_] = 0;
    }
    void Copy(blkdev_table_register_request *rq) {
        SetCode(rq->GetCode());
        dev_id_ = rq->GetDeviceID();
    }
    int GetDeviceID() {
        return dev_id_;
    }
};
}
#endif

struct labstor_blkdev_table_register_request {
    struct labstor_request header_;
    struct labstor_qtok_t kern_qtok_;
    int pathlen_;
    uint32_t dev_id_;
    char path_[];
};

struct labstor_blkdev_table_unregister_request {
    struct labstor_request header_;
    int dev_id_;
};



#endif //LABSTOR_BLKDEV_TABLE_H