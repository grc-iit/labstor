//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_GENERIC_POSIX_CLIENT_H
#define LABSTOR_GENERIC_POSIX_CLIENT_H

#include <generic_posix.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>
#include <mutex>

//TODO: Make this configurable
#define LABSTOR_FD_MIN 50000

namespace labstor::GenericPosix {
class Client : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
    bool is_initialized_;
    std::mutex lock_;
    int ns_id_;
    int fd_min_;
    int fd_start_;
public:
    Client() : labstor::Module(GENERIC_POSIX_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
        is_initialized_ = false;
    }
    inline void Initialize();
    int Open(const char *path, int oflag);
    int Close(int fd);
    ssize_t IOSync(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size);
    ssize_t Read(int fd, void *buf, size_t size) {
        return IOSync(labstor::GenericPosix::Ops::kRead, fd, buf, size);
    }
    ssize_t Write(int fd, void *buf, size_t size) {
        return IOSync(labstor::GenericPosix::Ops::kWrite, fd, buf, size);
    }
};
}

#endif //LABSTOR_GENERIC_POSIX_CLIENT_H
