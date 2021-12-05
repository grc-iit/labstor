//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_BLKDEV_TABLE_H
#define LABSTOR_BLKDEV_TABLE_H

#define BLKDEV_TABLE_MODULE_ID "BlkdevTable"

#define MAX_MOUNTED_BDEVS 64

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>

struct labstor_submit_blkdev_table_register_request {
    struct labstor_request header;
    uint32_t dev_id;
    char path[];
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

#endif //LABSTOR_BLKDEV_TABLE_H
