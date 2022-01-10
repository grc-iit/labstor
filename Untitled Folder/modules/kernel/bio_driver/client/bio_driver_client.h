//
// Created by lukemartinlogan on 12/5/21.
//

#ifndef LABSTOR_BIO_DRIVER_CLIENT_H
#define LABSTOR_BIO_DRIVER_CLIENT_H

#include <labstor/userspace/client/client.h>
#include <modules/kernel/bio_driver/bio_driver.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>

namespace labstor::BIODriver {

class Client: public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    uint32_t ns_id_;
public:
    Client() : labstor::Module(BIO_DRIVER_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Register();
    void IO(Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector);
    inline void Read(int dev_id, void *user_buf, size_t buf_size, size_t sector) {
        IO(Ops::kRead, dev_id, user_buf, buf_size, sector);
    }
    inline void Write(int dev_id, void *user_buf, size_t buf_size, size_t sector) {
        IO(Ops::kWrite, dev_id, user_buf, buf_size, sector);
    }

    labstor::ipc::qtok_t AIO(Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector);
    inline labstor::ipc::qtok_t ARead(int dev_id, void *user_buf, size_t buf_size, size_t sector) {
        return AIO(Ops::kRead, dev_id, user_buf, buf_size, sector);
    }
    inline labstor::ipc::qtok_t AWrite(int dev_id, void *user_buf, size_t buf_size, size_t sector) {
        return AIO(Ops::kWrite, dev_id, user_buf, buf_size, sector);
    }
};

}

#endif //LABSTOR_BIO_DRIVER_CLIENT_H
