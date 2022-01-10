//
// Created by lukemartinlogan on 12/5/21.
//

#ifndef LABSTOR_MQ_DRIVER_CLIENT_H
#define LABSTOR_MQ_DRIVER_CLIENT_H

#include <labstor/userspace/client/client.h>
#include <modules/kernel/mq_driver/mq_driver.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>

namespace labstor::MQDriver {

class Client: public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    uint32_t ns_id_;
public:
    Client() : labstor::Module(MQ_DRIVER_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Register();
    void IO(Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx);
    inline void Read(int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        IO(Ops::kRead, dev_id, user_buf, buf_size, sector, hctx);
    }
    inline void Write(int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        IO(Ops::kWrite, dev_id, user_buf, buf_size, sector, hctx);
    }

    labstor::ipc::qtok_t AIO(Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx);
    inline labstor::ipc::qtok_t ARead(int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        return AIO(Ops::kRead, dev_id, user_buf, buf_size, sector, hctx);
    }
    inline labstor::ipc::qtok_t AWrite(int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        return AIO(Ops::kWrite, dev_id, user_buf, buf_size, sector, hctx);
    }
};

}

#endif //LABSTOR_MQ_DRIVER_CLIENT_H
