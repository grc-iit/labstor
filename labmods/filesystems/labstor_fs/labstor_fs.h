//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_LABSTOR_FS_H
#define LABSTOR_LABSTOR_FS_H

#include <cstring>
#include <labstor/types/data_structures/shmem_request.h>
#include <labmods/filesystems/generic_posix/generic_posix.h>
#include <labmods/registrar/registrar.h>

#define LABFS_MODULE_ID "LabFS"

namespace labstor::LabFS {

struct register_request : public labstor::Registrar::register_request {
    bool has_fs_;
    labstor::id next_;
    void ConstructModuleStart(
            const std::string &module_id, const std::string &key, char *next_module, bool has_fs_) {
        labstor::Registrar::register_request::ConstructModuleStart(module_id, key);
        next_.copy(next_module);
        has_fs_ = has_fs_;
    }
};

struct mkfs_request : public labstor::ipc::request {
    size_t log_size_;
    size_t num_small_blocks_;
    void MkfsStart(size_t log_size, size_t num_small_blocks) {
    }
};

}

#endif //LABSTOR_LABSTOR_FS_H
