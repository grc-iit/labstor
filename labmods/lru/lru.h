//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_LABSTOR_FS_H
#define LABSTOR_LABSTOR_FS_H

#include <cstring>
#include <labstor/types/data_structures/shmem_request.h>
#include <labmods/generic_posix/generic_posix.h>
#include <labmods/registrar/registrar.h>

#define LABFS_MODULE_ID "LabFS"

namespace labstor::LRU {

struct register_request : public labstor::Registrar::register_request {
    labstor::id next_;
    void ConstructModuleStart(uint32_t ns_id, char *next_module) {
        ns_id_ = ns_id;
        code_ = static_cast<int>(GenericPosix::Ops::kInit);
        next_.copy(next_module);
    }
};

}

#endif //LABSTOR_LABSTOR_FS_H
