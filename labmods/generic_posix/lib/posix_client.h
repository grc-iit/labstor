//
// Created by lukemartinlogan on 3/10/22.
//

#ifndef LABSTOR_POSIX_CLIENT_H
#define LABSTOR_POSIX_CLIENT_H

#include <labmods/generic_posix/generic_posix.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>
#include <labstor/userspace/util/error.h>
#include <mutex>

namespace labstor::Posix {

class Client : public labstor::Module {
public:
    explicit Client(labstor::id &&module_id) : labstor::Module(module_id) {}
    virtual int Open(int fd, const char *path, int pathlen, int oflag) = 0;
    virtual int Close(int fd) = 0;
    virtual labstor::ipc::qtok_t AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size) = 0;
    virtual labstor::ipc::qtok_t AIO(labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size) = 0;
    virtual ssize_t IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size) = 0;
    virtual ssize_t IO(labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size) = 0;
};

}

#endif //LABSTOR_POSIX_CLIENT_H
