//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_SERVER_NAMESPACE_H
#define LABSTOR_SERVER_NAMESPACE_H

#include <vector>
#include <queue>

#include <labstor/constants/macros.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/types/shared_namespace.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/mpmc/shmem_ring_buffer.h>
#include <labstor/types/data_structures/mpmc/unordered_map/shmem_string_map.h>
#include <labstor/types/data_structures/shmem_string.h>

#include "macros.h"
#include "server.h"

#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

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
