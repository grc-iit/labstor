//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_BLOCK_FS_CLIENT_H
#define LABSTOR_BLOCK_FS_CLIENT_H

#include <labmods/filesystems/generic_posix/lib/posix_client.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>

namespace labstor::BlockFS {

class Client : public labstor::Posix::Client {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
    uint32_t ns_id_;
    std::unordered_map<int,labstor::GenericPosix::FILE*> fd_to_file_;
public:
    Client() : labstor::Posix::Client(BLOCKFS_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    void Initialize(labstor::ipc::request *rq) {}
    void Register(char *ns_key, char *next_module);
    int Open(int fd, const char *path, int pathlen, int oflag);
    int Close(int fd);
    labstor::ipc::qtok_t AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size);
    ssize_t IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size);
};

};

#endif //LABSTOR_BLOCK_FS_CLIENT_H
