//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_SERVER_NAMESPACE_H
#define LABSTOR_SERVER_NAMESPACE_H

#include <vector>
#include <queue>

#include <secure_shmem/netlink_client/shmem_user_netlink.h>
#include <labstor/constants/macros.h>
#include <labstor/types/basics.h>
#include <labstor/types/module.h>
#include <labstor/types/shmem_spinlock.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/shmem_string_map.h>
#include <labstor/types/data_structures/shmem_array.h>
#include <labstor/types/data_structures/shmem_string.h>
#include "macros.h"
#include "server.h"

namespace labstor::Server {

class Namespace {
private:
    labstor::GenericAllocator *alloc_;
    int region_id_;
    void *region_;
    labstor::ipc::SpinLock lock_;
    ShmemNetlinkClient shmem_;

    std::unordered_map<labstor::id, std::queue<labstor::Module*>> module_id_to_instance_;
    labstor::ipc::ring_buffer<uint32_t> ns_ids_;
    labstor::ipc::string_map key_to_ns_id_;
    labstor::ipc::array<uint32_t> shared_state_;
    std::vector<labstor::Module*> private_state_;
public:
    Namespace() {
        LABSTOR_CONFIGURATION_MANAGER_T labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
        uint32_t max_entries = labstor_config_->config_["namespace"]["max_entries"].as<uint32_t>();
        uint32_t shmem_size = labstor_config_->config_["namespace"]["shmem_kb"].as<uint32_t>() * 1024;
        uint32_t request_unit = labstor_config_->config_["namespace"]["shmem_request_unit"].as<uint32_t>();

        //Create a shared memory region
        region_id_ = shmem_.CreateShmem(shmem_size, true);
        if(region_id_ < 0) {
            throw SHMEM_CREATE_FAILED.format();
        }
        shmem_.GrantPidShmem(getpid(), region_id_);
        region_ = (char*)shmem_.MapShmem(region_id_, shmem_size);
        if(!region_) {
            throw MMAP_FAILED.format(strerror(errno));
        }

        //Initialize the shared memory space
        uint32_t remainder = shmem_size;
        void *section = region_;
        key_to_ns_id_.Init(section, labstor::ipc::string_map::GetSize(max_entries));
        remainder -= key_to_ns_id_.GetSize();
        section = key_to_ns_id_.GetNextSection();
        shared_state_.Init(section, labstor::ipc::array<uint32_t>::GetSize(max_entries));
        remainder -= shared_state_.GetSize();
        section = shared_state_.GetNextSection();

        //Create memory allocator on remaining memory
        labstor::ipc::shmem_allocator *alloc;
        alloc = new labstor::ipc::shmem_allocator();
        alloc->Init(section, remainder, request_unit);
        alloc_ = alloc;
    }

    ~Namespace() {
        if(alloc_) { delete alloc_; }
        for(auto module : private_state_) {
            delete module;
        }
        shmem_.FreeShmem(region_id_);
    }

    void *AllocateShmem(uint32_t size, uint32_t ns_id) {
        return nullptr;
    }

    void AddKey(labstor::ipc::string key, labstor::Module *module) {
        lock_.Lock();
        uint32_t ns_id;
        if(ns_ids_.Dequeue(ns_id)) {
            private_state_[ns_id] = module;
        } else {
            ns_id = private_state_.size();
            private_state_.emplace_back(module);
        }
        key_to_ns_id_.Set(key, ns_id);
        module_id_to_instance_[module->GetModuleID()].push(module);
        lock_.UnLock();
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
    labstor::Module *Get(uint32_t ns_id) { return private_state_[ns_id]; }

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
