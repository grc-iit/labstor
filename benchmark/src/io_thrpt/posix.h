//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_POSIX_H
#define LABSTOR_POSIX_H

#include "io_test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <aio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <errno.h>
#include <aio.h>
#include <labstor/types/thread_local.h>
#include <vector>

namespace labstor {

struct PosixIOThread {
    aiocb *cbs_;
    char *buf_;
    int fd_;
    PosixIOThread(char *path, int ops_per_thread, size_t block_size, size_t io_offset) {
        //Open file
        fd_ = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd_ < 0) {
            printf("Could not open/create file\n");
            exit(1);
        }
        buf_ = reinterpret_cast<char*>(aligned_alloc(4096, block_size));
        cbs_ = reinterpret_cast<aiocb*>(malloc(sizeof(aiocb)*ops_per_thread));
        memset(buf_, 0, block_size);
        lseek(fd_, io_offset, SEEK_SET);
    }
};

class PosixIO : public IOTest {
private:
    std::vector<PosixIOThread> thread_bufs_;
public:
    void Init(char *path, size_t block_size, size_t total_size, int nthreads, bool do_truncate) {
        AInit(path, block_size, total_size, 1, nthreads, do_truncate);
    }

    void AInit(char *path, size_t block_size, size_t total_size, int ops_per_batch, int nthreads, bool do_truncate) {
        IOTest::AInit(block_size, total_size, ops_per_batch, nthreads);
        //Open file & truncate
        int fd = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd < 0) {
            printf("Could not open/create file\n");
            exit(1);
        }
        if(do_truncate) {
            ftruncate(fd, GetTotalIO());
        }
        close(fd);
        //Store per-thread data
        for(int i = 0; i < nthreads_; ++i) {
            thread_bufs_.emplace_back(path, GetOpsPerBatch(), GetIOPerBatch(), i*GetIOPerThread());
        }
    }

    void Write() {
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixIOThread &thread = thread_bufs_[tid];
        if (write(thread.fd_, thread.buf_, block_size_) != block_size_) {
            printf("Error, could not write POSIX: %s\n", strerror(errno));
            exit(1);
        }
    }

    void Read() {
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixIOThread &thread = thread_bufs_[tid];
        if (read(thread.fd_, thread.buf_, block_size_) != block_size_) {
            printf("Error, could not read POSIX: %s\n", strerror(errno));
            exit(1);
        }
    }

    void AIO(int op) {
        struct aiocb *cb;
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixIOThread &thread = thread_bufs_[tid];
        for(int i = 0; i < GetOpsPerBatch(); ++i) {
            cb = thread.cbs_ + i;
            cb->aio_buf = thread.buf_ + i*block_size_;
            cb->aio_nbytes = block_size_;
            cb->aio_fildes = thread.fd_;
            cb->aio_lio_opcode = op;
        }
        if (lio_listio(LIO_WAIT, reinterpret_cast<aiocb *const *>(thread.cbs_), GetOpsPerBatch(), NULL) < 0) {
            printf("Error, could not read APOSIX: %s\n", strerror(errno));
            exit(1);
        }
    }

    void ARead() {
        AIO(LIO_READ);
    }

    void AWrite() {
        AIO(LIO_WRITE);
    }
};

}

#endif //LABSTOR_POSIX_H
