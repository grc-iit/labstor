
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

#ifndef LABSTOR_CLIENT_NAMESPACE_H
#define LABSTOR_CLIENT_NAMESPACE_H

#include <vector>

#include <labstor/userspace/types/shared_namespace.h>
#include <labstor/constants/macros.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/module_manager.h>
#include <labstor/userspace/client/ipc_manager.h>

#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include "labstor/types/data_structures/shmem_ring_buffer.h"
#include <labstor/types/data_structures/unordered_map/shmem_string_map.h>
#include <labstor/userspace/types/module.h>
#include <labmods/registrar/client/registrar_client.h>
#include <labmods/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

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
        std::string path = LABSTOR_REGISTRAR->GetModulePath(ns_id);
        TRACEPOINT("PATH", path)
        labstor::id module_id = module_manager_->UpdateModule(path);
        TRACEPOINT("MODULE ID", module_id.key_)
        labstor::Module *module = module_manager_->GetModuleConstructor(module_id)();
        module->SetNamespaceID(ns_id);
        module->Initialize(ns_id);
        RegisterPrivateState(ns_id, module);
        return reinterpret_cast<T*>(module);
    }

    template<typename T>
    T* LoadClientModule(std::string key) {
        uint32_t ns_id = LABSTOR_REGISTRAR->GetNamespaceID(key);
        if(ns_id == -1) {
            return nullptr;
        }
        return LoadClientModule<T>(ns_id);
    }

    void DeleteModule(std::string key) {
    }
};


}

#endif //LABSTOR_CLIENT_NAMESPACE_H