//
// Created by lukemartinlogan on 6/7/21.
//

#include <labstor/interfaces/posix.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <search.h>
#include <assert.h>
#include <libgen.h>
#include <pthread.h>
#include <stdint.h>
#include <limits.h>

#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <aio.h>

#include <generic_posix/client/client.h>

/**
 * PROTOTYPES
 * */

#define WRAPPER_FUN(x) x
#define REAL_FUN(x) __real_##x
#define FNAME_TYPE(T, fname, ...) T (*)(__VA_ARGS__)
#define FORWARD_DECL(T, fname, ...) \
    T (*REAL_FUN(fname))(__VA_ARGS__) = NULL;
#define GETFUN(T, fname, ...) \
    if(!REAL_FUN(fname)) { REAL_FUN(fname) = (FNAME_TYPE(T, fname, __VA_ARGS__))dlsym(RTLD_NEXT, #fname); }

FORWARD_DECL(ssize_t, read, int fd, void *buf, size_t count)
FORWARD_DECL(ssize_t, write, int fd, const void *buf, size_t count)
FORWARD_DECL(ssize_t, pread, int fd, void *buf, size_t count, off_t offset)
FORWARD_DECL(ssize_t, pwrite, int fd, const void *buf, size_t count, off_t offset)
FORWARD_DECL(ssize_t, pread64, int fd, void *buf, size_t count, off64_t offset)
FORWARD_DECL(ssize_t, pwrite64, int fd, const void *buf, size_t count, off64_t offset)
FORWARD_DECL(ssize_t, readv, int fd, const struct iovec *iov, int iovcnt)
FORWARD_DECL(ssize_t, preadv, int fd, const struct iovec *iov, int iovcnt, off_t offset)
FORWARD_DECL(ssize_t, preadv64, int fd, const struct iovec *iov, int iovcnt, off64_t offset)
FORWARD_DECL(ssize_t, preadv2, int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
FORWARD_DECL(ssize_t, preadv64v2, int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags)
FORWARD_DECL(ssize_t, writev, int fd, const struct iovec *iov, int iovcnt)
FORWARD_DECL(ssize_t, pwritev, int fd, const struct iovec *iov, int iovcnt, off_t offset)
FORWARD_DECL(ssize_t, pwritev64, int fd, const struct iovec *iov, int iovcnt, off64_t offset)
FORWARD_DECL(ssize_t, pwritev2, int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
FORWARD_DECL(ssize_t, pwritev64v2, int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags)
FORWARD_DECL(int, aio_read, struct aiocb *aiocbp)
FORWARD_DECL(int, aio_write, struct aiocb *aiocbp)
FORWARD_DECL(int, aio_read64, struct aiocb64 *aiocbp)
FORWARD_DECL(int, aio_write64, struct aiocb64 *aiocbp)
FORWARD_DECL(ssize_t, aio_return, struct aiocb *aiocbp)
FORWARD_DECL(ssize_t, aio_return64, struct aiocb64 *aiocbp)
FORWARD_DECL(int, lio_listio, int mode, struct aiocb *const aiocb_list[], int nitems, struct sigevent *sevp)
FORWARD_DECL(int, lio_listio64, int mode, struct aiocb64 *const aiocb_list[], int nitems, struct sigevent *sevp)

size_t write_size = 0;

/**
 * POSIX FUNCTIONS
 * */

ssize_t WRAPPER_FUN(open)(int fd, void *buf, size_t count)
{
    GenericFS_Client::
    return REAL_FUN(read)(fd, buf, count);
}

ssize_t WRAPPER_FUN(read)(int fd, void *buf, size_t count)
{
    //Submit request to user-space server
    return REAL_FUN(read)(fd, buf, count);
}

ssize_t WRAPPER_FUN(write)(int fd, const void *buf, size_t count)
{
    return REAL_FUN(write)(fd, buf, count);
}

ssize_t WRAPPER_FUN(pread)(int fd, void *buf, size_t count, off_t offset)
{
    return REAL_FUN(pread)(fd, buf, count, offset);
}

ssize_t WRAPPER_FUN(pwrite)(int fd, const void *buf, size_t count, off_t offset)
{
    return REAL_FUN(pwrite)(fd, buf, count, offset);
}

ssize_t WRAPPER_FUN(pread64)(int fd, void *buf, size_t count, off64_t offset)
{
    return REAL_FUN(pread64)(fd, buf, count, offset);
}

ssize_t WRAPPER_FUN(pwrite64)(int fd, const void *buf, size_t count, off64_t offset)
{
    return REAL_FUN(pwrite64)(fd, buf, count, offset);
}

ssize_t WRAPPER_FUN(readv)(int fd, const struct iovec *iov, int iovcnt)
{
    return REAL_FUN(readv)(fd, iov, iovcnt);
}

ssize_t WRAPPER_FUN(preadv)(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
    return REAL_FUN(preadv)(fd, iov, iovcnt, offset);
}

ssize_t WRAPPER_FUN(preadv64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset)
{
    return REAL_FUN(preadv64)(fd, iov, iovcnt, offset);
}

ssize_t WRAPPER_FUN(preadv2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
{
    return REAL_FUN(preadv2)(fd, iov, iovcnt, offset, flags);
}

ssize_t WRAPPER_FUN(preadv64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags)
{
    return REAL_FUN(preadv64v2)(fd, iov, iovcnt, offset, flags);
}

ssize_t WRAPPER_FUN(writev)(int fd, const struct iovec *iov, int iovcnt)
{
    return REAL_FUN(writev)(fd, iov, iovcnt);
}

ssize_t WRAPPER_FUN(pwritev)(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
    return REAL_FUN(pwritev)(fd, iov, iovcnt, offset);
}

ssize_t WRAPPER_FUN(pwritev64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset)
{
    return REAL_FUN(pwritev64)(fd, iov, iovcnt, offset);
}

ssize_t WRAPPER_FUN(pwritev2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
{
    return REAL_FUN(pwritev2)(fd, iov, iovcnt, offset, flags);
}

ssize_t WRAPPER_FUN(pwritev64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags)
{
    return REAL_FUN(pwritev64v2)(fd, iov, iovcnt, offset, flags);
}

int WRAPPER_FUN(aio_read)(struct aiocb *aiocbp)
{
    return REAL_FUN(aio_read)(aiocbp);
}

int WRAPPER_FUN(aio_write)(struct aiocb *aiocbp)
{
    return REAL_FUN(aio_write)(aiocbp);
}

int WRAPPER_FUN(aio_read64)(struct aiocb64 *aiocbp)
{
    return REAL_FUN(aio_read64)(aiocbp);
}

int WRAPPER_FUN(aio_write64)(struct aiocb64 *aiocbp)
{
    return REAL_FUN(aio_write64)(aiocbp);
}

ssize_t WRAPPER_FUN(aio_return)(struct aiocb *aiocbp)
{
    return REAL_FUN(aio_return)(aiocbp);
}

ssize_t WRAPPER_FUN(aio_return64)(struct aiocb64 *aiocbp)
{
    return REAL_FUN(aio_return64)(aiocbp);
}

int WRAPPER_FUN(lio_listio)(int mode, struct aiocb *const aiocb_list[], int nitems, struct sigevent *sevp)
{
    return REAL_FUN(lio_listio)(mode, aiocb_list, nitems, sevp);
}

int WRAPPER_FUN(lio_listio64)(int mode, struct aiocb64 *const aiocb_list[], int nitems, struct sigevent *sevp)
{
    return REAL_FUN(lio_listio64)(mode, aiocb_list, nitems, sevp);
}
