//
// Created by lukemartinlogan on 3/26/22.
//

#ifndef LABSTOR_BLOCK_CLIENT_H
#define LABSTOR_BLOCK_CLIENT_H

#include "labstor/userspace/types/module.h"
#include "labmods/storage_api/block/block.h"

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
