//
// Created by lukemartinlogan on 3/26/22.
//

#ifndef LABSTOR_BLOCK_H
#define LABSTOR_BLOCK_H

#include <labstor/types/data_structures/shmem_request.h>

namespace labstor::GenericBlock {

enum class Ops {
    kRegister,
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
