//
// Created by lukemartinlogan on 3/9/22.
//

#ifndef LABSTOR_SHARED_NAMESPACE_H
#define LABSTOR_SHARED_NAMESPACE_H

#include <vector>
#include <queue>

#include <labstor/constants/constants.h>
#include <labstor/constants/macros.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/module.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/mpmc/shmem_ring_buffer.h>
#include <labstor/types/data_structures/mpmc/unordered_map/shmem_string_map.h>
#include <labstor/types/data_structures/shmem_string.h>

namespace labstor {

class Namespace {
protected:
    labstor::GenericAllocator *shmem_alloc_;
    int region_id_;
    uint32_t region_size_;
    void *region_;
    uint32_t max_entries_;

    std::unordered_map<labstor::id, std::queue<labstor::Module*>> module_id_to_instance_;
    labstor::ipc::mpmc::ring_buffer<uint32_t> ns_ids_;
    labstor::ipc::mpmc::string_map key_to_ns_id_;
    std::vector<labstor::Module*> private_state_;
public:
    inline void GetSharedRegion(uint32_t &region_id, uint32_t &region_size, uint32_t &max_entries) {
        region_id = region_id_;
        region_size = region_size_;
        max_entries = max_entries_;
    }

    inline void RegisterPrivateState(uint32_t ns_id, labstor::Module *module) {
        if(ns_id > private_state_.size()) {
            private_state_.resize(ns_id+1);
        }
        private_state_[ns_id] = module;
    }

    inline uint32_t AddKey(labstor::ipc::string key, labstor::Module *module) {
        uint32_t ns_id;
        if(ns_ids_.Dequeue(ns_id)) {
            private_state_[ns_id] = module;
        } else {
            ns_id = private_state_.size();
            private_state_.emplace_back(module);
        }
        TRACEPOINT(key.ToString(), key.Hash(),  ns_id);
        if(!key_to_ns_id_.Set(key, ns_id)) {
            FAILED_TO_SET_NAMESPACE_KEY.format(ns_id)->print();
        }
        module_id_to_instance_.emplace(module->GetModuleID(), std::move(std::queue<labstor::Module*>()));
        module_id_to_instance_[module->GetModuleID()].push(module);
        return ns_id;
    }
    inline uint32_t AddKey(labstor::id key, labstor::Module *module) {
        return AddKey(labstor::ipc::string(key.key_, shmem_alloc_), module);
    }
    inline uint32_t AddKey(std::string key, labstor::Module *module) {
        return AddKey(labstor::ipc::string(key, shmem_alloc_), module);
    }

    inline void DeleteKey(labstor::ipc::string key) {
        labstor::Module *module = RemoveKey(key);
        if(module == nullptr) { return; }
        //TODO: delete module;
    }
    inline void RenameKey(labstor::ipc::string old_key, labstor::ipc::string new_key) {
        labstor::Module *module = RemoveKey(old_key);
        if(module == nullptr) { return; }
        AddKey(new_key, module);
    }
    inline bool GetIfExists(labstor::ipc::string key, uint32_t &ns_id) {
        TRACEPOINT(key.ToString(), key.Hash())
        return key_to_ns_id_.Find(key, ns_id);
    }
    inline uint32_t Get(labstor::ipc::string key) {
        uint32_t ns_id;
        TRACEPOINT(key.ToString(), key.Hash())
        if(key_to_ns_id_.Find(key, ns_id)) {
            return ns_id;
        }
        return -1;
    }
    template<typename T=labstor::Module>
    inline T* Get(uint32_t ns_id) {
        if(0 <= ns_id && ns_id < private_state_.size()) {
            return reinterpret_cast<T*>(private_state_[ns_id]);
        }
        return nullptr;
    }
    uint32_t GetNamespaceID(std::string key) {
        uint32_t ns_id;
        if(!key_to_ns_id_.Find(labstor::ipc::string(key), ns_id)) {
            return LABSTOR_INVALID_NAMESPACE_KEY;
        }
        return ns_id;
    }

    inline std::queue<labstor::Module*>& AllModuleInstances(labstor::id module_id) {
        return module_id_to_instance_[module_id];
    }
private:
    labstor::Module* RemoveKey(labstor::ipc::string key) {
        return nullptr;
    }
};

}

#endif //LABSTOR_SHARED_NAMESPACE_H
