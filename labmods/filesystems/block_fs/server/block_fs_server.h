//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_BLOCK_FS_SERVER_H
#define LABSTOR_BLOCK_FS_SERVER_H

#include <labmods/filesystems/generic_posix/generic_posix.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>
#include <labstor/types/data_structures/unordered_map/shmem_int_map.h>

namespace labstor::BlockFS {
class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
    uint32_t next_module_;
public:
    Server() : labstor::Module(BLOCKFS_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    inline void Initialize(labstor::ipc::request *rq) override;
    bool ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) override;
    inline bool Open(labstor::queue_pair *qp, labstor::GenericPosix::open_request *client_rq, labstor::credentials *creds);
    inline bool Close(labstor::queue_pair *qp, labstor::GenericPosix::close_request *client_rq, labstor::credentials *creds);
    inline bool IO(labstor::queue_pair *qp, labstor::GenericPosix::io_request *client_rq, labstor::credentials *creds);
};
}

#endif //LABSTOR_BLOCK_FS_SERVER_H
