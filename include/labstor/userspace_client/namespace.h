//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_CLIENT_NAMESPACE_H
#define LABSTOR_CLIENT_NAMESPACE_H

#include <vector>

#include <labstor/constants/macros.h>
#include <labstor/userspace_client/macros.h>
#include <labstor/userspace_client/module_manager.h>
#include <labstor/userspace_client/ipc_manager.h>

#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_string_map.h>
#include <labstor/types/data_structures/shmem_array.h>
#include <labstor/types/data_structures/shmem_ring_buffer.h>
#include <labstor/types/module.h>

namespace labstor::Client {

struct Namespace {
    bool is_decentralized_;
    int region_id_;
    void *region_;
    labstor::GenericAllocator *alloc_;
    labstor::ipc::ring_buffer<uint32_t> ns_ids_;
    labstor::ipc::string_map key_to_ns_id_;
    labstor::ipc::array<uint32_t> shared_state_;
    std::vector<labstor::Module*> private_state_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;

    Namespace() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        is_decentralized_ = false;
    }

    void AcquireNamespace(int region_id, uint32_t region_size) {
        is_decentralized_ = true;
    }

    uint32_t GetNamespaceID(std::string key) {
        if(is_decentralized_) {
            uint32_t *id = key_to_ns_id_.Find(key);
            if(id == nullptr) { throw INVALID_NAMESPACE_ENTRY.format(); }
            return *id;
        }
    }

    void* GetSharedState(uint32_t ns_id) {
        if(is_decentralized_) {
            return LABSTOR_REGION_ADD(shared_state_[ns_id], region_);
        }
    }
};

}

#endif //LABSTOR_CLIENT_NAMESPACE_H
