
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

#ifndef LABSTOR_SERVER_NAMESPACE_H
#define LABSTOR_SERVER_NAMESPACE_H

#include <vector>
#include <queue>

#include <labstor/constants/macros.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/types/shared_namespace.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include "labstor/types/data_structures/shmem_ring_buffer.h"
#include <labstor/types/data_structures/unordered_map/shmem_string_map.h>
#include <labstor/types/data_structures/shmem_string.h>

#include "macros.h"
#include "server.h"

#include <labmods/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

namespace labstor::Server {

class Namespace : public labstor::Namespace {
public:
    Namespace();
    void Init();

    ~Namespace() {
        LABSTOR_KERNEL_SHMEM_ALLOC_T shmem = LABSTOR_KERNEL_SHMEM_ALLOC;
        if(shmem_alloc_) { delete shmem_alloc_; }
        for(auto module : private_state_) {
            delete module;
        }
        shmem->FreeShmem(region_id_);
    }
};

}

#endif //LABSTOR_SERVER_NAMESPACE_H