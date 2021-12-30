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
    INVALID_LABSTOR_FD = -1350,
    LABSTOR_GENERIC_FS_PATH_NOT_FOUND = -1351,
};

namespace labstor::GenericPosix {
enum class Ops {
    kRegister,
    kMount,
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
#ifdef __cplusplus
    inline void Start(int ns_id, const char *path, int oflags) {
        header_.SetNamespaceID(ns_id);
        header_.SetOp(static_cast<int>(labstor::GenericPosix::Ops::kOpen));
        oflags_ = oflags;
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
#endif
};

struct generic_posix_io_request {
    struct labstor_request header_;
    int fd_;
    void *buf_;
    ssize_t size_;
#ifdef __cplusplus
    inline void Start(int ns_id, labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size) {
        header_.SetNamespaceID(ns_id);
        header_.SetOp(static_cast<int>(op));
        fd_ = fd;
        buf_ = buf;
        size_ = size;
    }
    inline void Complete(ssize_t size) {
        size_ = size;
    }
    inline ssize_t GetSize() {
        return size_;
    }
#endif
};

struct generic_posix_close_request {
    struct labstor_request header_;
    int fd_;
#ifdef __cplusplus
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
#endif
};

struct generic_posix_poll_io_request : public labstor::ipc::poll_request_single<generic_posix_io_request> {
    inline void Init(labstor::ipc::queue_pair *qp, generic_posix_io_request *reply_rq, labstor::ipc::qtok_t &poll_qtok) {
        int op = static_cast<int>(labstor::GenericPosix::Ops::kIOComplete);
        labstor::ipc::poll_request_single<generic_posix_io_request>::Init(qp, reply_rq, poll_qtok, op);
    }
};

#endif //LABSTOR_GENERIC_POSIX_H
