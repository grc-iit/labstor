//
// Created by lukemartinlogan on 11/26/21.
//

#include <labstor/userspace_server/namespace.h>
#include <modules/kernel/secure_shmem/netlink_client/shmem_user_netlink.h>
#include <modules/live_upgrade/live_upgrade.h>
#include <modules/registrar/server/registrar_server.h>

labstor::Server::Namespace::Namespace() {
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    uint32_t max_entries = labstor_config_->config_["namespace"]["max_entries"].as<uint32_t>();
    uint32_t max_collisions = labstor_config_->config_["namespace"]["max_collisions"].as<uint32_t>();
    uint32_t request_unit = labstor_config_->config_["namespace"]["shmem_request_unit"].as<uint32_t>() * SizeType::BYTES;
    uint32_t shmem_size = labstor_config_->config_["namespace"]["shmem_kb"].as<uint32_t>() * SizeType::KB;
    region_size_ = shmem_size;

    //Create a shared memory region
    region_id_ = shmem_.CreateShmem(shmem_size, true);
    if(region_id_ < 0) {
        throw SHMEM_CREATE_FAILED.format();
    }
    shmem_.GrantPidShmem(getpid(), region_id_);
    region_ = shmem_.MapShmem(region_id_, shmem_size);
    if(!region_) {
        throw MMAP_FAILED.format(strerror(errno));
    }

    //Initialize the namespace tables
    uint32_t remainder = shmem_size;
    void *section = region_;
    key_to_ns_id_.Init(section, labstor::ipc::string_map::GetSize(max_entries, max_collisions), max_collisions);
    remainder -= key_to_ns_id_.GetSize();
    section = key_to_ns_id_.GetNextSection();
    shared_state_.Init(section, labstor::ipc::array<uint32_t>::GetSize(max_entries));
    remainder -= shared_state_.GetSize();
    section = shared_state_.GetNextSection();

    //Create memory allocator on remaining memory
    labstor::ipc::shmem_allocator *alloc;
    alloc = new labstor::ipc::shmem_allocator();
    alloc->Init(section, remainder, request_unit);
    shmem_alloc_ = alloc;

    //Add the registration module
    AddKey(labstor::ipc::string("Registrar", shmem_alloc_), new labstor::Registrar::Server());
}