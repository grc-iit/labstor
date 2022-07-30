
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

#ifndef LABSTOR_MQ_DRIVER_CLIENT_H
#define LABSTOR_MQ_DRIVER_CLIENT_H

#include <labstor/userspace/client/client.h>
#include <labmods/mq_driver/mq_driver.h>
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
    int dev_id_;
public:
    Client() : labstor::Module(MQ_DRIVER_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Initialize(int ns_id) override {}
    void Register(YAML::Node config) override;
    void IO(Ops op, void *user_buf, size_t buf_size, size_t sector, int hctx);
    inline void Read(void *user_buf, size_t buf_size, size_t sector, int hctx) {
        IO(Ops::kRead, user_buf, buf_size, sector, hctx);
    }
    inline void Write(void *user_buf, size_t buf_size, size_t sector, int hctx) {
        IO(Ops::kWrite, user_buf, buf_size, sector, hctx);
    }

    labstor::ipc::qtok_t AIO(Ops op, void *user_buf, size_t buf_size, size_t sector, int hctx);
    inline labstor::ipc::qtok_t ARead(void *user_buf, size_t buf_size, size_t sector, int hctx) {
        return AIO(Ops::kRead, user_buf, buf_size, sector, hctx);
    }
    inline labstor::ipc::qtok_t AWrite(void *user_buf, size_t buf_size, size_t sector, int hctx) {
        return AIO(Ops::kWrite, user_buf, buf_size, sector, hctx);
    }

    int GetNumHWQueues();
};

}

#endif //LABSTOR_MQ_DRIVER_CLIENT_H