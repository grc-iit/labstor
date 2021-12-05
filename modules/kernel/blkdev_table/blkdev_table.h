//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_BLKDEV_TABLE_H
#define LABSTOR_BLKDEV_TABLE_H

#define BLKDEV_TABLE_MODULE_ID "BlkdevTable"

#define MAX_MOUNTED_BDEVS 64

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>

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
        kUnregisterBdev
    };
}
#endif

struct labstor_submit_blkdev_table_register_request {
    struct labstor_request header_;
    int pathlen_;
    uint32_t dev_id_;
    char path_[];
#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t pathlen) {
        return sizeof(labstor_submit_blkdev_table_register_request) + pathlen + 1;
    }
    inline void Init(int ns_id, const char *path, int pathlen, int devid) {
        header_.op_ = static_cast<int>(labstor::BlkdevTable::Ops::kRegisterBdev);
        header_.ns_id_ = ns_id;
        pathlen_ = pathlen;
        memcpy(path_, path, pathlen);
        path_[pathlen_] = 0;
    }
    inline void Init(int ns_id, labstor_submit_blkdev_table_register_request *rq) {
        header_.ns_id_ = ns_id;
        header_.op_ = rq->header_.op_;
        pathlen_ = rq->pathlen_;
        memcpy(path_, rq->path_, pathlen_);
        path_[pathlen_] = 0;
    }
#endif
};

struct labstor_complete_blkdev_table_register_request {
    struct labstor_request header;
};

struct labstor_submit_blkdev_table_unregister_request {
    struct labstor_request header;
    int dev_id;
};

struct labstor_complete_blkdev_table_unregister_request {
    struct labstor_request header;
};



#endif //LABSTOR_BLKDEV_TABLE_H
