
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

#ifndef LABSTOR_NO_OP_IOSCHED_SERVER_H
#define LABSTOR_NO_OP_IOSCHED_SERVER_H

#include "labstor/userspace/server/server.h"
#include <labmods/generic_block/generic_block.h>
#include "labstor/userspace/types/module.h"
#include "labmods/no_op/no_op.h"
#include "labstor/userspace/server/macros.h"
#include "labstor/userspace/server/ipc_manager.h"
#include "labstor/userspace/server/namespace.h"


namespace labstor::iosched::NoOp {

class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
    int next_module_, num_hw_queues_, queue_depth_;
public:
    Server() : labstor::Module(NO_OP_IOSCHED_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    bool ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
    bool Initialize(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) override;
    bool IO(labstor::queue_pair *qp, labstor::GenericBlock::io_request *client_rq, labstor::credentials *creds);
};

}

#endif //LABSTOR_NO_OP_IOSCHED_SERVER_H