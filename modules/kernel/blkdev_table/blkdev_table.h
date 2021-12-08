//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_BLKDEV_TABLE_H
#define LABSTOR_BLKDEV_TABLE_H

#define BLKDEV_TABLE_MODULE_ID "BlkdevTable"

#define MAX_MOUNTED_BDEVS 64
#define BDEV_ACCESS_FLAGS FMODE_READ | FMODE_WRITE | FMODE_PREAD | FMODE_PWRITE //| FMODE_EXCL
#define MAX_PAGES_PER_GET 128

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
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
        kRegisterBdevComplete,
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
    struct labstor_submit_blkdev_table_register_request wrapper;
#ifdef __cplusplus
    void SetDeviceID(labstor_complete_blkdev_table_register_request *rq) {
        wrapper.dev_id_ = rq->GetDeviceID();
        wrapper.header_.ns_id_ = rq->GetReturnCode();
    }
    int GetDeviceID() {
        return wrapper.dev_id_;
    }
    int GetReturnCode() {
        return wrapper.header_.ns_id_;
    }
#endif
};

struct labstor_poll_blkdev_table_register {
    struct labstor_request header_;
    struct labstor_qtok_t kqtok_;
    struct labstor_qtok_t uqtok_;
#ifdef __cplusplus
    void Init(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq, labstor::ipc::qtok_t &qtok) {
        header_.ns_id_ = rq->header_.ns_id_;
        header_.op_ = static_cast<int>(labstor::BlkdevTable::Ops::kRegisterBdevComplete);
        kqtok_ = qtok;
        uqtok_.qid = qp->GetQid();
        uqtok_.req_id = rq->header_.req_id_;
    }
#endif
};

struct labstor_submit_blkdev_table_unregister_request {
    struct labstor_request header_;
    int dev_id_;
};

struct labstor_complete_blkdev_table_unregister_request {
    struct labstor_request header_;
};



#endif //LABSTOR_BLKDEV_TABLE_H
