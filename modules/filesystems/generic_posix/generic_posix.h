//
// Created by lukemartinlogan on 12/12/21.
//

#ifndef LABSTOR_GENERIC_POSIX_H
#define LABSTOR_GENERIC_POSIX_H

#include <cstring>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_poll.h>

#define GENERIC_POSIX_MODULE_ID "GenericPosix"

enum GENERIC_FS_RETURN_CODES {
    LABSTOR_GENERIC_FS_SUCCESS = 0,
    LABSTOR_GENERIC_FS_INVALID_FD = -1350,
    LABSTOR_GENERIC_FS_PATH_NOT_FOUND = -1351,
    LABSTOR_GENERIC_FS_INVALID_PATH = -1352,
};

namespace labstor::GenericPosix {
enum class Ops {
    kRegister,
    kInit,
    kOpen,
    kClose,
    kRead,
    kWrite,
    kIOComplete
};
}

struct generic_posix_open_request {
    struct labstor_request header_;
    int oflags_;
    int fd_;
    char path_[];
    inline void ClientInit(int ns_id, const char *path, int oflags, int fd) {
        header_.SetNamespaceID(ns_id);
        header_.SetOp(static_cast<int>(labstor::GenericPosix::Ops::kOpen));
        oflags_ = oflags;
        fd_ = fd;
        strcpy(path_, path);
    }
    inline void Complete(int code) {
        header_.SetCode(code);
    }
    inline void SetFD(int fd) {
        fd_ = fd;
    }
    inline int GetFD() {
        return fd_;
    }
    inline int GetCode() {
        return header_.GetCode();
    }
};

struct generic_posix_close_request {
    struct labstor_request header_;
    int fd_;
    inline void Start(int ns_id, int fd) {
        header_.SetNamespaceID(ns_id);
        header_.SetOp(static_cast<int>(labstor::GenericPosix::Ops::kClose));
        fd_ = fd;
    }
    inline void Complete(int ret) {
        header_.SetCode(ret);
    }
    inline int GetCode() {
        return header_.GetCode();
    }
    inline void SetFD(int fd) {
        fd_ = fd;
    }
    inline int GetFD() {
        return fd_;
    }
};

struct generic_posix_passthrough_request {
    struct labstor_request header_;
    int fd_;
    inline void SetFD(int fd) {
        fd_ = fd;
    }
    inline int GetFD() {
        return fd_;
    }
    inline void Complete(int code) {
        header_.SetCode(code);
    }
};

struct generic_posix_io_request : generic_posix_passthrough_request {
    void *buf_;
    ssize_t size_;
    inline void Start(int ns_id, labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size) {
        header_.SetNamespaceID(ns_id);
        header_.SetOp(static_cast<int>(op));
        fd_ = fd;
        buf_ = buf;
        size_ = size;
    }
    inline void Complete(ssize_t size, int code) {
        size_ = size;
        generic_posix_passthrough_request::Complete(code);
    }
    inline ssize_t GetSize() {
        return size_;
    }
};

#endif //LABSTOR_GENERIC_POSIX_H
