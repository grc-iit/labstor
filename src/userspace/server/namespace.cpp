
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

#include <labstor/userspace/server/server.h>
#include <labstor/constants/debug.h>
#include <labstor/userspace/server/namespace.h>
#include <labmods/secure_shmem/netlink_client/secure_shmem_client_netlink.h>
#include <labmods/registrar/server/registrar_server.h>

labstor::Server::Namespace::Namespace() {
    AUTO_TRACE("")
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    uint32_t max_entries = labstor_config_->config_["namespace"]["max_entries"].as<uint32_t>();
    uint32_t max_collisions = labstor_config_->config_["namespace"]["max_collisions"].as<uint32_t>();
    uint32_t request_unit = labstor_config_->config_["namespace"]["shmem_request_unit"].as<uint32_t>() * SizeType::BYTES;
    uint32_t shmem_size = labstor_config_->config_["namespace"]["shmem_kb"].as<uint32_t>() * SizeType::KB;
    region_size_ = shmem_size;

    //Create a shared memory region
    TRACEPOINT(max_entries, max_collisions, request_unit, shmem_size)
    LABSTOR_KERNEL_SHMEM_ALLOC_T shmem = LABSTOR_KERNEL_SHMEM_ALLOC;
    region_id_ = shmem->CreateShmem(shmem_size, true);
    if(region_id_ < 0) {
        throw SHMEM_CREATE_FAILED.format();
    }
    shmem->GrantPidShmem(getpid(), region_id_);
    region_ = shmem->MapShmem(region_id_, shmem_size);
    if(!region_) {
        throw MMAP_FAILED.format(strerror(errno));
    }
    TRACEPOINT(region_id_)

    //Initialize the namespace tables
    TRACEPOINT("NamespaceTables")
    uint32_t remainder = shmem_size;
    void *section = region_;
    ns_ids_.Init(section, labstor::ipc::mpmc::ring_buffer<uint32_t>::GetSize(max_entries));
    remainder -= ns_ids_.GetSize();
    section = ns_ids_.GetNextSection();
    key_to_ns_id_.Init(region_, section, labstor::ipc::mpmc::string_map::GetSize(max_entries), 0, 16);
    remainder -= key_to_ns_id_.GetSize();
    section = key_to_ns_id_.GetNextSection();
    max_entries_ = max_entries;
    TRACEPOINT("NamespaceTables")

    TRACEPOINT("SIZES", ns_ids_.GetSize(), key_to_ns_id_.GetSize(), region_id_)

    //Create memory allocator on remaining memory for key names
    labstor::ipc::shmem_allocator *alloc;
    alloc = new labstor::ipc::shmem_allocator();
    alloc->Init(region_, section, remainder, request_unit);
    shmem_alloc_ = alloc;
}

void labstor::Server::Namespace::Init() {
    //Add the registration module
    AddKey(labstor::ipc::string("Registrar", shmem_alloc_), new labstor::Registrar::Server());
}