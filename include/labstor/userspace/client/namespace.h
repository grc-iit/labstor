//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_CLIENT_NAMESPACE_H
#define LABSTOR_CLIENT_NAMESPACE_H

#include <vector>

#include <labstor/userspace/types/shared_namespace.h>
#include <labstor/constants/macros.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/module_manager.h>
#include <labstor/userspace/client/ipc_manager.h>

#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include <labstor/types/data_structures/shmem_array.h>
#include "labstor/types/data_structures/shmem_ring_buffer.h"
#include <labstor/types/data_structures/unordered_map/shmem_string_map.h>
#include <labstor/userspace/types/module.h>
#include <labmods/registrar/client/registrar_client.h>
#include <labmods/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

namespace labstor::Client {

struct Namespace : public labstor::Namespace {
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_MODULE_MANAGER_T module_manager_;

    Namespace() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        module_manager_ = LABSTOR_MODULE_MANAGER;
    }

    void Attach(int region_id, uint32_t region_size) {
        region_id_ = region_id;
        region_ = labstor::kernel::netlink::ShmemClient::MapShmem(region_id, region_size);
        void *section = region_;
        ns_ids_.Attach(section);
        section = ns_ids_.GetNextSection();
        key_to_ns_id_.Attach(region_, section);
        section = ns_ids_.GetNextSection();

        TRACEPOINT("SIZES", ns_ids_.GetSize(), key_to_ns_id_.GetSize(), region_id_)

        labstor::ipc::shmem_allocator *alloc;
        alloc = new labstor::ipc::shmem_allocator();
        alloc->Attach(region_, section);
        shmem_alloc_ = alloc;
    }

    template<typename T>
    T* LoadClientModule(uint32_t ns_id) {
        AUTO_TRACE(ns_id)
        labstor::Registrar::Client client;
        std::string path = client.GetModulePath(ns_id);
        TRACEPOINT("PATH", path)
        labstor::id module_id = module_manager_->UpdateModule(path);
        TRACEPOINT("MODULE ID", module_id.key_)
        labstor::Module *module = module_manager_->GetModuleConstructor(module_id)();
        module->Initialize(ns_id);
        RegisterPrivateState(ns_id, module);
        return reinterpret_cast<T*>(module);
    }
};

}

#endif //LABSTOR_CLIENT_NAMESPACE_H
