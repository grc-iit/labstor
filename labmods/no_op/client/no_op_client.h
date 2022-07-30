
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

#ifndef LABSTOR_NO_OP_IOSCHED_CLIENT_H
#define LABSTOR_NO_OP_IOSCHED_CLIENT_H

#include "labstor/userspace/client/client.h"
#include "labmods/no_op/no_op.h"
#include "labstor/constants/macros.h"
#include "labstor/constants/constants.h"
#include "labstor/userspace/types/module.h"
#include "labstor/userspace/client/macros.h"
#include "labstor/userspace/client/ipc_manager.h"
#include "labstor/userspace/client/namespace.h"
#include <labmods/generic_block/client/generic_block_client.h>

namespace labstor::iosched::NoOp {

class Client: public labstor::GenericBlock::Client {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    Client() : labstor::GenericBlock::Client(NO_OP_IOSCHED_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }
    void Register(YAML::Node config) override;
    void Initialize(int ns_id) override {}
    labstor::ipc::qtok_t AIO(void *buf, size_t size, size_t off, labstor::GenericBlock::Ops op) override;
};

}

#endif //LABSTOR_NO_OP_IOSCHED_CLIENT_H