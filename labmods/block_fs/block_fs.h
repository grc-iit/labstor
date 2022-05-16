//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_LABSTOR_FS_H
#define LABSTOR_LABSTOR_FS_H

#include <cstring>
#include <labstor/types/data_structures/shmem_request.h>
#include <labmods/generic_posix/generic_posix.h>
#include <labmods/registrar/registrar.h>

#define BLOCKFS_MODULE_ID "BlockFS"

#define SMALL_BLOCK_SIZE (4*(1<<10))
#define LARGE_BLOCK_SIZE (128*(1<<10))

namespace labstor::BlockFS {

struct register_request : labstor::ipc::request {
    labstor::id next_;
    void ConstructModuleStart(uint32_t ns_id, std::string next_module) {
        ns_id_ = ns_id;
        code_ = static_cast<int>(GenericPosix::Ops::kInit);
        next_.copy(next_module);
    }
};

}

#endif //LABSTOR_LABSTOR_FS_H
