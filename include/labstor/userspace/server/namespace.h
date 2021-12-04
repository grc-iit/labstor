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
#include <labstor/userspace/types/shmem_spinlock.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/shmem_string_map.h>
#include <labstor/types/data_structures/array/shmem_array_uint32_t.h>
#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_uint32_t.h>
#include <labstor/types/data_structures/shmem_string.h>

#include "macros.h"
#include "server.h"

#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

namespace labstor::Server {

class Namespace {
private:
    labstor::GenericAllocator *shmem_alloc_;
    int region_id_;
    uint32_t region_size_;
    void *region_;
    labstor::ipc::SpinLock lock_;
    ShmemNetlinkClient shmem_;

    std::unordered_map<labstor::id, std::queue<labstor::Module*>> module_id_to_instance_;
    labstor::ipc::ring_buffer_uint32_t ns_ids_;
    labstor::ipc::string_map key_to_ns_id_;
    labstor::ipc::array_uint32_t shared_state_;
    std::vector<labstor::Module*> private_state_;
public:
    Namespace();
    void Init();

    ~Namespace() {
        if(shmem_alloc_) { delete shmem_alloc_; }
        for(auto module : private_state_) {
            delete module;
        }
        shmem_.FreeShmem(region_id_);
    }

    inline void *AllocateShmem(uint32_t size, uint32_t ns_id) {
        return nullptr;
    }

    inline uint32_t AddKey(labstor::ipc::string key, labstor::Module *module) {
        uint32_t ns_id;
        if(ns_ids_.Dequeue(ns_id)) {
            private_state_[ns_id] = module;
        } else {
            ns_id = private_state_.size();
            private_state_.emplace_back(module);
        }
        TRACEPOINT("labstor::Server::AddKey", key.data_, ns_id);
        key_to_ns_id_.Set(key, ns_id);
        module_id_to_instance_[module->GetModuleID()].push(module);
        return ns_id;
    }

    inline uint32_t AddKey(labstor::id key, labstor::Module *module) {
        return AddKey(labstor::ipc::string(key.key, shmem_alloc_), module);
    }

    inline uint32_t AddKey(std::string key, labstor::Module *module) {
        return AddKey(labstor::ipc::string(key, shmem_alloc_), module);
    }

    inline void DeleteKey(labstor::ipc::string key) {
        labstor::Module *module = RemoveKey(key);
        if(module == nullptr) { return; }
        delete module;
    }
    inline void RenameKey(labstor::ipc::string old_key, labstor::ipc::string new_key) {
        labstor::Module *module = RemoveKey(old_key);
        if(module == nullptr) { return; }
        AddKey(new_key, module);
    }
    inline void GetNamespaceRegion(uint32_t &region_id, uint32_t &region_size) {
        region_id = region_id_;
        region_size = region_size_;
    }
    inline uint32_t Get(labstor::ipc::string key) { return key_to_ns_id_[key]; }
    inline labstor::Module *Get(uint32_t ns_id) { return private_state_[ns_id]; }

    inline std::queue<labstor::Module*>& AllModuleInstances(labstor::id module_id) {
        return module_id_to_instance_[module_id];
    }
private:
    labstor::Module* RemoveKey(labstor::ipc::string key) {
        return nullptr;
    }
};

}

#endif //LABSTOR_SERVER_NAMESPACE_H
