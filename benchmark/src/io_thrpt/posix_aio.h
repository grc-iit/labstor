//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_POSIX_AIO_H
#define LABSTOR_POSIX_AIO_H

#include "unix_file_based.h"
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

struct PosixAIOThread {
    aiocb64 *cbs_;
    struct aiocb64 **cb_list_;
    char *buf_;
    int fd_;
    size_t io_offset_;
    PosixAIOThread(char *path, int ops_per_thread, size_t block_size, size_t io_offset) {
        //Open file
        fd_ = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd_ < 0) {
            printf("Could not open/create file\n");
            exit(1);
        }
        buf_ = reinterpret_cast<char*>(aligned_alloc(4096, block_size));
        cbs_ = reinterpret_cast<aiocb64*>(calloc(sizeof(aiocb), ops_per_thread));
        cb_list_ = reinterpret_cast<aiocb64**>(calloc(sizeof(aiocb*), ops_per_thread));
        for(int i = 0; i < ops_per_thread; ++i) {
            cb_list_[i] = cbs_ + i;
        }
        memset(buf_, 0, block_size);
        io_offset_ = io_offset;
    }
};

class PosixAIO : public UnixFileBasedIOTest {
private:
    std::vector<PosixAIOThread> thread_bufs_;
public:
    void Init(char *path, size_t block_size, size_t total_size, int ops_per_batch, int nthreads, bool do_truncate) {
        UnixFileBasedIOTest::Init(path, block_size, total_size, ops_per_batch, nthreads, do_truncate);
        //Store per-thread data
        for(int i = 0; i < nthreads_; ++i) {
            thread_bufs_.emplace_back(path, GetOpsPerBatch(), GetIOPerBatch(), i*GetIOPerThread());
        }
    }
    void AIO(int op) {
        struct aiocb64 *cb;
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixAIOThread &thread = thread_bufs_[tid];
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            cb = thread.cbs_ + i;
            cb->aio_buf = thread.buf_ + i*block_size_;
            cb->aio_offset = thread.io_offset_ + i*block_size_;
            cb->aio_nbytes = block_size_;
            cb->aio_fildes = thread.fd_;
            cb->aio_lio_opcode = op;
        }
        if (lio_listio64(LIO_WAIT, thread.cb_list_, GetOpsPerBatch(), NULL) < 0) {
            printf("Error, APOSIX IO failed: %s\n", strerror(errno));
            printf("Ops per batch: %lu\n", GetOpsPerBatch());
            exit(1);
        }
    }

    void Read() {
        AIO(LIO_READ);
    }

    void Write() {
        AIO(LIO_WRITE);
    }
};

}

#endif //LABSTOR_POSIX_AIO_H
