//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_CLIENT_NAMESPACE_H
#define LABSTOR_CLIENT_NAMESPACE_H

#include <vector>

#include <labstor/constants/macros.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/module_manager.h>
#include <labstor/userspace/client/ipc_manager.h>

#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_array.h>
#include <labstor/types/data_structures/mpmc/shmem_ring_buffer.h>
#include <labstor/types/data_structures/mpmc/unordered_map/shmem_string_map.h>
#include <labstor/userspace/types/module.h>
#include <modules/registrar/client/registrar_client.h>

namespace labstor::Client {

struct Namespace {
    bool is_decentralized_;
    int region_id_;
    void *region_;
    labstor::GenericAllocator *alloc_;
    labstor::ipc::mpmc::ring_buffer<uint32_t> ns_ids_;
    labstor::ipc::mpmc::string_map key_to_ns_id_;
    labstor::ipc::array<uint32_t> shared_state_;
    std::vector<labstor::Module*> private_state_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    labstor::Registrar::Client registrar_;

    Namespace() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        is_decentralized_ = false;
    }

    void AcquireNamespace(int region_id, uint32_t region_size) {
        is_decentralized_ = true;
    }

    uint32_t GetNamespaceID(std::string key) {
        uint32_t ns_id;
        if(is_decentralized_ && key_to_ns_id_.Find(labstor::ipc::string(key), ns_id)) {
            return ns_id;
        } else {
            return registrar_.GetNamespaceID(key);
        }
        return LABSTOR_INVALID_NAMESPACE_KEY;
    }

    void* GetSharedState(uint32_t ns_id) {
        if(is_decentralized_) {
            uint32_t off =  shared_state_[ns_id];
            if(off == -1) {
                return nullptr;
            }
            return LABSTOR_REGION_ADD(off, region_);
        }
        return nullptr;
    }
};

}

#endif //LABSTOR_CLIENT_NAMESPACE_H
