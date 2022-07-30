
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

#ifndef LABSTOR_MQ_DRIVER_H
#define LABSTOR_MQ_DRIVER_H

#include "labstor/types/basics.h"
#include "labstor/types/data_structures/shmem_request.h"
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include "labmods/registrar/registrar.h"
#include <labmods/generic_block/generic_block.h>

#define NO_OP_IOSCHED_MODULE_ID "NO_OP"

namespace labstor::iosched::NoOp {

struct register_request : public labstor::Registrar::register_request {
    labstor::id next_;
    void ConstructModuleStart(uint32_t ns_id, const std::string &next_module) {
        ns_id_ = ns_id;
        code_ = static_cast<int>(GenericBlock::Ops::kInit);
        next_.copy(next_module);
    }
};

}

#endif //LABSTOR_MQ_DRIVER_H