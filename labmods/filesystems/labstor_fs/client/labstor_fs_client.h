//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_LABSTOR_FS_CLIENT_H
#define LABSTOR_LABSTOR_FS_CLIENT_H

#include <labmods/filesystems/labstor_fs/labstor_fs.h>
#include <labmods/filesystems/generic_posix/lib/posix_client.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>

namespace labstor::LabFS {

class Client : public labstor::Posix::Client {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
public:
    Client() : labstor::Posix::Client(LABFS_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    void Register(YAML::Node config) override;
    int Open(int fd, const char *path, int pathlen, int oflag);
    int Close(int fd);
    labstor::ipc::qtok_t AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size);
    ssize_t IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size);
};

};

#endif //LABSTOR_LABSTOR_FS_CLIENT_H
