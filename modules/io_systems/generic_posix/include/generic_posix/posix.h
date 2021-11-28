//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_POSIX_H
#define LABSTOR_POSIX_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <labstor/interface.h>
#include <unistd.h>

enum op {
    POSIX_OPEN_RQ,
    POSIX_READ_RQ,
    POSIX_WRITE_RQ,
    POSIX_CLOSE_RQ
};

struct posix_open_request {
    char *path;
    int flags;
    mode_t mode;
};

struct posix_io_request {
    int fd;
    void *buf;
    ssize_t count;
};

struct posix_request : public struct request {
    int op;
    union {
        struct posix_open_request open;
        struct posix_io_request io;
    };
};

struct posix_ops {
    int (*open)(const char *pathname, int flags, mode_t mode);
    labstor::off_t (*lseek)(int fd, labstor::off_t offset, int whence);
    int (*close)(int fd);

    ssize_t (*read)(int fd, void *buf, ssize_t count);
    ssize_t (*readv)(int fd, const struct iovec *iov, int iovcnt);
    ssize_t (*pread)(int fd, void *buf, size_t count, labstor::off_t offset);
    ssize_t (*pread64)(int fd, void *buf, size_t count, off64_t offset);
    ssize_t (*preadv)(int fd, const struct iovec *iov, int iovcnt, labstor::off_t offset);
    ssize_t (*preadv64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset);
    ssize_t (*preadv2)(int fd, const struct iovec *iov, int iovcnt, labstor::off_t offset, int flags);
    ssize_t (*preadv64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags);

    ssize_t (*write)(int fd, void *buf, ssize_t count);
    ssize_t (*pwrite)(int fd, void *buf, size_t count, labstor::off_t offset);
    ssize_t (*pwrite64)(int fd, void *buf, size_t count, off64_t offset);
    ssize_t (*writev)(int fd, const struct iovec *iov, int iovcnt);
    ssize_t (*pwritev)(int fd, const struct iovec *iov, int iovcnt, labstor::off_t offset);
    ssize_t (*pwritev64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset);
    ssize_t (*pwritev2)(int fd, const struct iovec *iov, int iovcnt, labstor::off_t offset, int flags);
    ssize_t (*pwritev64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags);

    int (*aio_read)(struct aiocb *aiocbp);
    int (*aio_read64)(struct aiocb64 *aiocbp);
    int (*aio_write)(struct aiocb *aiocbp);
    int (*aio_write64)(struct aiocb64 *aiocbp);
    ssize_t (*aio_return)(struct aiocb *aiocbp);
    ssize_t (*aio_return64)(struct aiocb64 *aiocbp);

    int lio_listio(int mode, struct aiocb *const aiocb_list[], int nitems, struct sigevent *sevp);
    int lio_listio64(int mode, struct aiocb64 *const aiocb_list[], int nitems, struct sigevent *sevp);
};

#endif //LABSTOR_POSIX_H
