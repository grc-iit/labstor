//
// Created by lukemartinlogan on 6/7/21.
//

#define _XOPEN_SOURCE 500
#define _GNU_SOURCE

#include <mpi.h>

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
#include <pmemcpy/util/trace.h>
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

#define DRAM_BW (68.0*(1ul<<30))
#define DRAM_LATENCY 100
#define PMEM_READ_BW (30.0*(1ul<<30))
#define PMEM_WRITE_BW (7.8*(1ul<<30))
#define PMEM_READ_LATENCY 300
#define PMEM_WRITE_LATENCY 125
inline void nsleep(long delay) {
    struct timespec req;
    req.tv_nsec = delay;
    req.tv_sec = 0;
    nanosleep(&req, NULL);
}
#define READ_PENALTY(size) ((PMEM_READ_LATENCY - DRAM_LATENCY) + ((size)/PMEM_READ_BW - (size)/DRAM_BW))
#define WRITE_PENALTY(size) ((PMEM_WRITE_LATENCY - DRAM_LATENCY) + ((size)/PMEM_WRITE_BW - (size)/DRAM_BW))
#define ADD_READ_PENALTY(size) nsleep(READ_PENALTY(size))
#define ADD_WRITE_PENALTY(size) nsleep(WRITE_PENALTY(size))
inline size_t IOV_COUNT(const struct iovec *iov, int iovcnt) {
    size_t count = 0;
    for(int i = 0; i < iovcnt; ++i) { count += iov->iov_len; }
    return count;
}
#define AIO_COUNT(aiocbp) aiocbp->aio_nbytes


size_t netsize = 0;

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

FORWARD_DECL(size_t, fwrite, const void *ptr, size_t size, size_t nmemb, FILE *stream)
FORWARD_DECL(size_t, fread, void *ptr, size_t size, size_t nmemb, FILE *stream)
FORWARD_DECL(int, fputc, int c, FILE *stream)
FORWARD_DECL(int, putw, int w, FILE *stream)
FORWARD_DECL(int, fgetc, FILE *stream)
FORWARD_DECL(int, getw, FILE *stream)

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
 * STDIO FUNCTIONS
 * */

size_t WRAPPER_FUN(fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    AUTO_TRACE("fwrite: size={}, penalty={}ns", pmemcpy::SizeType(size*nmemb, pmemcpy::SizeType::MB), READ_PENALTY(size*nmemb));
    ADD_WRITE_PENALTY(size*nmemb);
    GETFUN(size_t, fwrite, const void *ptr, size_t size, size_t nmemb, FILE *stream)
    return REAL_FUN(fwrite)(ptr, size, nmemb, stream);
}

size_t WRAPPER_FUN(fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    AUTO_TRACE("fread: size={}, penalty={}ns", pmemcpy::SizeType(size*nmemb, pmemcpy::SizeType::MB), READ_PENALTY(size*nmemb));
    ADD_READ_PENALTY(size*nmemb);
    GETFUN(size_t, fread, void *ptr, size_t size, size_t nmemb, FILE *stream)
    return REAL_FUN(fread)(ptr, size, nmemb, stream);
}

int WRAPPER_FUN(fputc)(int c, FILE *stream) {
    AUTO_TRACE("fputc");
    GETFUN(int, fputc, int c, FILE *stream)
    return REAL_FUN(fputc)(c, stream);
}
int WRAPPER_FUN(putw)(int w, FILE *stream) {
    AUTO_TRACE("putw");
    GETFUN(int, putw, int w, FILE *stream)
    return REAL_FUN(putw)(w, stream);
}
int WRAPPER_FUN(fgetc)(FILE *stream) {
    AUTO_TRACE("fgetc");
    GETFUN(int, fgetc, FILE *stream)
    return REAL_FUN(fgetc)(stream);
}
int WRAPPER_FUN(getw)(FILE *stream) {
    AUTO_TRACE("getw");
    GETFUN(int, getw, FILE *stream)
    return REAL_FUN(getw)(stream);
}

/**
 * POSIX FUNCTIONS
 * */


ssize_t WRAPPER_FUN(read)(int fd, void *buf, size_t count)
{
    //AUTO_TRACE("read: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(count, pmemcpy::SizeType::MB), READ_PENALTY(count));
    ADD_READ_PENALTY(count);
    GETFUN(ssize_t, read, int fd, void *buf, size_t count)
    return REAL_FUN(read)(fd, buf, count);
}

ssize_t WRAPPER_FUN(write)(int fd, const void *buf, size_t count)
{
    netsize += count;
    AUTO_TRACE("write: fd={}, size={} vs {}, total_size={}, penalty={}ns", fd, pmemcpy::SizeType(count, pmemcpy::SizeType::MB), count, netsize, WRITE_PENALTY(count));
    ADD_WRITE_PENALTY(count);
    GETFUN(ssize_t, write, int fd, const void *buf, size_t count)
    return REAL_FUN(write)(fd, buf, count);
}

ssize_t WRAPPER_FUN(pread)(int fd, void *buf, size_t count, off_t offset)
{
    AUTO_TRACE("pread: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(count, pmemcpy::SizeType::MB), READ_PENALTY(count));
    ADD_READ_PENALTY(count);
    GETFUN(ssize_t, pread, int fd, void *buf, size_t count, off_t offset)
    return REAL_FUN(pread)(fd, buf, count, offset);
}

ssize_t WRAPPER_FUN(pwrite)(int fd, const void *buf, size_t count, off_t offset)
{
    AUTO_TRACE("pwrite: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(count, pmemcpy::SizeType::MB), WRITE_PENALTY(count));
    ADD_WRITE_PENALTY(count);
    GETFUN(ssize_t, pwrite, int fd, const void *buf, size_t count, off_t offset)
    return REAL_FUN(pwrite)(fd, buf, count, offset);
}

ssize_t WRAPPER_FUN(pread64)(int fd, void *buf, size_t count, off64_t offset)
{
    AUTO_TRACE("pread64: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(count, pmemcpy::SizeType::MB), READ_PENALTY(count));
    ADD_READ_PENALTY(count);
    GETFUN(ssize_t, pread64, int fd, void *buf, size_t count, off64_t offset)
    return REAL_FUN(pread64)(fd, buf, count, offset);
}

ssize_t WRAPPER_FUN(pwrite64)(int fd, const void *buf, size_t count, off64_t offset)
{
    AUTO_TRACE("pwrite64: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(count, pmemcpy::SizeType::MB), WRITE_PENALTY(count));
    ADD_WRITE_PENALTY(count);
    GETFUN(ssize_t, pwrite64, int fd, const void *buf, size_t count, off64_t offset)
    return REAL_FUN(pwrite64)(fd, buf, count, offset);
}

ssize_t WRAPPER_FUN(readv)(int fd, const struct iovec *iov, int iovcnt)
{
    AUTO_TRACE("readv: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), READ_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_READ_PENALTY(IOV_COUNT(iov, iovcnt));
    return REAL_FUN(readv)(fd, iov, iovcnt);
}

ssize_t WRAPPER_FUN(preadv)(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
    AUTO_TRACE("preadv: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), READ_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_READ_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, readv, int fd, const struct iovec *iov, int iovcnt)
    return REAL_FUN(preadv)(fd, iov, iovcnt, offset);
}

ssize_t WRAPPER_FUN(preadv64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset)
{
    AUTO_TRACE("preadv64: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), READ_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_READ_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, preadv, int fd, const struct iovec *iov, int iovcnt, off_t offset)
    return REAL_FUN(preadv64)(fd, iov, iovcnt, offset);
}

ssize_t WRAPPER_FUN(preadv2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
{
    AUTO_TRACE("preadv2: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), READ_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_READ_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, preadv64, int fd, const struct iovec *iov, int iovcnt, off64_t offset)
    return REAL_FUN(preadv2)(fd, iov, iovcnt, offset, flags);
}

ssize_t WRAPPER_FUN(preadv64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags)
{
    AUTO_TRACE("preadv64v2: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), READ_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_READ_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, preadv2, int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
    return REAL_FUN(preadv64v2)(fd, iov, iovcnt, offset, flags);
}

ssize_t WRAPPER_FUN(writev)(int fd, const struct iovec *iov, int iovcnt)
{
    AUTO_TRACE("writev: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), WRITE_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_WRITE_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, writev, int fd, const struct iovec *iov, int iovcnt)
    return REAL_FUN(writev)(fd, iov, iovcnt);
}

ssize_t WRAPPER_FUN(pwritev)(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
    AUTO_TRACE("pwritev: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), WRITE_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_WRITE_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, pwritev, int fd, const struct iovec *iov, int iovcnt, off_t offset)
    return REAL_FUN(pwritev)(fd, iov, iovcnt, offset);
}

ssize_t WRAPPER_FUN(pwritev64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset)
{
    AUTO_TRACE("pwritev64: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), WRITE_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_WRITE_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, pwritev64, int fd, const struct iovec *iov, int iovcnt, off64_t offset)
    return REAL_FUN(pwritev64)(fd, iov, iovcnt, offset);
}

ssize_t WRAPPER_FUN(pwritev2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
{
    AUTO_TRACE("pwritev2: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), WRITE_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_WRITE_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, pwritev2, int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags)
    return REAL_FUN(pwritev2)(fd, iov, iovcnt, offset, flags);
}

ssize_t WRAPPER_FUN(pwritev64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags)
{
    AUTO_TRACE("pwritev64v2: fd={}, size={}, penalty={}ns", fd, pmemcpy::SizeType(IOV_COUNT(iov, iovcnt), pmemcpy::SizeType::MB), WRITE_PENALTY(IOV_COUNT(iov, iovcnt)));
    ADD_WRITE_PENALTY(IOV_COUNT(iov, iovcnt));
    GETFUN(ssize_t, pwritev64v2, int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags)
    return REAL_FUN(pwritev64v2)(fd, iov, iovcnt, offset, flags);
}

int WRAPPER_FUN(aio_read)(struct aiocb *aiocbp)
{
    AUTO_TRACE("aio_read: size={}, penalty={}ns", pmemcpy::SizeType(AIO_COUNT(aiocbp), pmemcpy::SizeType::MB), READ_PENALTY(AIO_COUNT(aiocbp)));
    ADD_READ_PENALTY(AIO_COUNT(aiocbp));
    GETFUN(int, aio_read, struct aiocb *aiocbp)
    return REAL_FUN(aio_read)(aiocbp);
}

int WRAPPER_FUN(aio_write)(struct aiocb *aiocbp)
{
    AUTO_TRACE("aio_write: size={}, penalty={}ns", pmemcpy::SizeType(AIO_COUNT(aiocbp), pmemcpy::SizeType::MB), WRITE_PENALTY(AIO_COUNT(aiocbp)));
    ADD_WRITE_PENALTY(AIO_COUNT(aiocbp));
    GETFUN(int, aio_write, struct aiocb *aiocbp)
    return REAL_FUN(aio_write)(aiocbp);
}

int WRAPPER_FUN(aio_read64)(struct aiocb64 *aiocbp)
{
    AUTO_TRACE("aio_read64: size={}, penalty={}ns", pmemcpy::SizeType(AIO_COUNT(aiocbp), pmemcpy::SizeType::MB), READ_PENALTY(AIO_COUNT(aiocbp)));
    ADD_READ_PENALTY(AIO_COUNT(aiocbp));
    GETFUN(int, aio_read64, struct aiocb64 *aiocbp)
    return REAL_FUN(aio_read64)(aiocbp);
}

int WRAPPER_FUN(aio_write64)(struct aiocb64 *aiocbp)
{
    AUTO_TRACE("aio_write64: size={}, penalty={}ns", pmemcpy::SizeType(AIO_COUNT(aiocbp), pmemcpy::SizeType::MB), WRITE_PENALTY(AIO_COUNT(aiocbp)));
    ADD_WRITE_PENALTY(AIO_COUNT(aiocbp));
    GETFUN(int, aio_write64, struct aiocb64 *aiocbp)
    return REAL_FUN(aio_write64)(aiocbp);
}

ssize_t WRAPPER_FUN(aio_return)(struct aiocb *aiocbp)
{
    AUTO_TRACE("aio_return");
    GETFUN(ssize_t, aio_return, struct aiocb *aiocbp)
    return REAL_FUN(aio_return)(aiocbp);
}

ssize_t WRAPPER_FUN(aio_return64)(struct aiocb64 *aiocbp)
{
    AUTO_TRACE("aio_return64");
    GETFUN(ssize_t, aio_return64, struct aiocb64 *aiocbp)
    return REAL_FUN(aio_return64)(aiocbp);
}

int WRAPPER_FUN(lio_listio)(int mode, struct aiocb *const aiocb_list[], int nitems, struct sigevent *sevp)
{
    AUTO_TRACE("lio_listio");
    GETFUN(int, lio_listio, int mode, struct aiocb *const aiocb_list[], int nitems, struct sigevent *sevp)
    return REAL_FUN(lio_listio)(mode, aiocb_list, nitems, sevp);
}

int WRAPPER_FUN(lio_listio64)(int mode, struct aiocb64 *const aiocb_list[], int nitems, struct sigevent *sevp)
{
    AUTO_TRACE("lio_listio64");
    GETFUN(int, lio_listio64, int mode, struct aiocb64 *const aiocb_list[], int nitems, struct sigevent *sevp)
    return REAL_FUN(lio_listio64)(mode, aiocb_list, nitems, sevp);
}
