//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_IOSCHED_MANUAL_H
#define LABSTOR_IOSCHED_MANUAL_H

#include <labstor/types/data_stuctures/shmem_request.h>

namespace labstor::iosched {

enum class ManualOps {
    kWriteOp,
    kReadOp,
    kPoll
};

struct mq_submit_request : public labstor::ipc::request {
    int dev_id_;
    void *buf_;
    size_t buf_size_;
    size_t lba_;
    int hctx_;

    mq_submit_request() = default;

    inline void Init(uint32_t ns_id, ManualOps op, int dev_id, void *buf, size_t buf_size, size_t lba, int hctx) {
        dev_id_ = dev_id;
        ns_id_ = ns_id;
        op_ = static_cast<uint32_t>(op);
        buf_ = buf;
        buf_size_ = buf_size,
        lba_ = lba;
        hctx_ = hctx;
    }
};

struct mq_complete_request : public labstor::ipc::request {
    int status_;

    mq_complete_request() = default;

    inline void Init(int status) {
        status_ = status;
    }
};

}

#endif //LABSTOR_IOSCHED_MANUAL_H
