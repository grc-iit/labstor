//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_LABSTOR_FS_H
#define LABSTOR_LABSTOR_FS_H

#include <cstring>
#include <labstor/types/data_structures/shmem_request.h>
#include <labmods/filesystems/generic_posix/generic_posix.h>
#include <labmods/registrar/registrar.h>

#define BLOCKFS_MODULE_ID "BlockFS"

namespace labstor::BlockFS {

struct register_request : public labstor::Registrar::register_request {
    labstor::id next_;
    void ConstructModuleStart(
            const std::string &module_id, const std::string &key, char *next_module) {
        labstor::Registrar::register_request::ConstructModuleStart(module_id, key);
        next_.copy(next_module);
    }
};

}

#endif //LABSTOR_LABSTOR_FS_H
