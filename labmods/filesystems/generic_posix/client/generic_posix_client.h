//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_GENERIC_POSIX_CLIENT_H
#define LABSTOR_GENERIC_POSIX_CLIENT_H

#include <generic_posix.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>
#include <labstor/userspace/util/error.h>
#include <mutex>

//TODO: Make this configurable
#define LABSTOR_FD_MIN 50000
#define LABSTOR_PATH_PREFIX "lab::"
#define LABSTOR_MAX_FDS_PER_THREAD 1000
#define LABSTOR_INVALID_FD -1

namespace labstor::GenericPosix {

const Error TOO_MANY_FDS(5000, "Too many file descriptors allocated by thread");

struct FDAllocator {
    int min_fd_, alloced_fds_, max_fds_;
    labstor::ipc::mpmc::ring_buffer<int> free_fds_;
    FDAllocator(int min_fd, char *region, size_t fd_alloc_size, int max_fds_per_thread) {
        free_fds_.Init(region, fd_alloc_size, max_fds_per_thread);
        min_fd_ = min_fd;
        alloced_fds_ = 0;
        max_fds_ = max_fds_per_thread;
    }
    int Allocate() {
        int fd;
        TRACEPOINT((size_t)free_fds_.GetRegion())
        if(free_fds_.Dequeue(fd)) {
            return fd;
        }
        fd = min_fd_ + alloced_fds_;
        if(fd >= min_fd_ + max_fds_) {
            throw TOO_MANY_FDS.format();
        }
        return fd;
    }
    void Free(int fd) {
        free_fds_.Enqueue(fd);
    }
};

class Client : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
    bool is_initialized_;
    std::mutex lock_;
    int fd_min_;
    std::string prefix_;
    std::vector<FDAllocator> fds_;
    std::unordered_map<int, uint32_t> fd_to_ns_id_;
public:
    Client() : labstor::Module(GENERIC_POSIX_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
        is_initialized_ = false;
        size_t fd_alloc_size = labstor::ipc::mpmc::ring_buffer<int>::GetSize(LABSTOR_MAX_FDS_PER_THREAD);
        char *region = (char*)malloc( fd_alloc_size * ipc_manager_->GetNumCPU());
        fds_.reserve(ipc_manager_->GetNumCPU());
        for(int i = 0; i < ipc_manager_->GetNumCPU(); ++i) {
            region += fd_alloc_size;
            fds_.emplace_back(LABSTOR_FD_MIN + i*LABSTOR_MAX_FDS_PER_THREAD, region, fd_alloc_size, LABSTOR_MAX_FDS_PER_THREAD);
        }
        LABSTOR_ERROR_HANDLE_START()
            Link();
        LABSTOR_ERROR_HANDLE_END()
    }
    void Initialize(int ns_id) {}
    inline bool IsInitialized() {
        return is_initialized_;
    }
    int Register();
    void Link();
    int Open(const char *path, int oflag);
    int Close(int fd);
    int AllocateFD() {
        TRACEPOINT(fds_.size(), ipc_manager_->GetNumCPU())
        return fds_[labstor::ThreadLocal::GetTid()].Allocate();
    }
    void FreeFD(int fd) {
        return fds_[labstor::ThreadLocal::GetTid()].Free(fd);
    }
    labstor::ipc::qtok_t AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size);
    labstor::ipc::qtok_t ARead(int fd, void *buf, size_t off, ssize_t size) {
        return AIO(labstor::GenericPosix::Ops::kRead, fd, buf, off, size);
    }
    labstor::ipc::qtok_t AWrite(int fd, void *buf, size_t off, ssize_t size) {
        return AIO(labstor::GenericPosix::Ops::kWrite, fd, buf, off, size);
    }

    labstor::ipc::qtok_t AIO(labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size);
    labstor::ipc::qtok_t ARead(int fd, void *buf, ssize_t size) {
        return AIO(labstor::GenericPosix::Ops::kRead, fd, buf, size);
    }
    labstor::ipc::qtok_t AWrite(int fd, void *buf, ssize_t size) {
        return AIO(labstor::GenericPosix::Ops::kWrite, fd, buf, size);
    }

    ssize_t IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size);
    ssize_t Read(int fd, void *buf, size_t off, ssize_t size) {
        return IO(labstor::GenericPosix::Ops::kRead, fd, buf, off, size);
    }
    ssize_t Write(int fd, void *buf, size_t off, ssize_t size) {
        return IO(labstor::GenericPosix::Ops::kWrite, fd, buf, off, size);
    }

    ssize_t IO(labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size);
    ssize_t Read(int fd, void *buf, ssize_t size) {
        return IO(labstor::GenericPosix::Ops::kRead, fd, buf, size);
    }
    ssize_t Write(int fd, void *buf, ssize_t size) {
        return IO(labstor::GenericPosix::Ops::kWrite, fd, buf, size);
    }
};
}

#endif //LABSTOR_GENERIC_POSIX_CLIENT_H
