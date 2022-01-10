//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_POSIX_H
#define LABSTOR_POSIX_H

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
#include <vector>

#include <labstor/types/thread_local.h>

namespace labstor {

struct PosixIOThread {
    char *buf_;
    int fd_;
    size_t io_offset_;
    PosixIOThread(char *path, int ops_per_thread, size_t block_size, size_t io_offset) {
        //Open file
        fd_ = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd_ < 0) {
            printf("Could not open/create file\n");
            exit(1);
        }
        buf_ = reinterpret_cast<char*>(aligned_alloc(4096, block_size));
        memset(buf_, 0, block_size);
        lseek(fd_, io_offset, SEEK_SET);
        io_offset_ = io_offset;
    }
};

class PosixIO : public UnixFileBasedIOTest {
private:
    std::vector<PosixIOThread> thread_bufs_;
public:
    void Init(char *path, size_t block_size, size_t total_size, int ops_per_batch, int nthreads, bool do_truncate) {
        UnixFileBasedIOTest::Init(path, block_size, total_size, ops_per_batch, nthreads, do_truncate);
        //Store per-thread data
        for(int i = 0; i < nthreads_; ++i) {
            thread_bufs_.emplace_back(path, GetOpsPerBatch(), GetIOPerBatch(), i*GetIOPerThread());
        }
    }

    void Write() {
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixIOThread &thread = thread_bufs_[tid];
        lseek(thread.fd_, thread.io_offset_, SEEK_SET);
        int ret = write(thread.fd_, thread.buf_, block_size_);
        if (ret != (int)block_size_) {
            printf("Error, could not write POSIX: %s\n", strerror(errno));
            exit(1);
        }
    }

    void Read() {
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixIOThread &thread = thread_bufs_[tid];
        int ret = read(thread.fd_, thread.buf_, block_size_);
        if(ret != (int)block_size_) {
            printf("Error, could not read POSIX: %s\n", strerror(errno));
            exit(1);
        }
    }
};

}

#endif //LABSTOR_POSIX_H
