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

struct PosixIOThread : public UnixFileBasedIOThread {
    PosixIOThread(char *path, size_t block_size) : UnixFileBasedIOThread(path, block_size) {}
};

class PosixIO : public UnixFileBasedIOTest {
private:
    std::vector<PosixIOThread> thread_bufs_;
public:
    PosixIO() = default;

    void Init(char *path, bool do_truncate, labstor::Generator *generator) {
        UnixFileBasedIOTest::Init(path, do_truncate, generator);
        //Store per-thread data
        for(int i = 0; i < GetNumThreads(); ++i) {
            thread_bufs_.emplace_back(path,GetBlockSizeBytes());
        }
    }

    void Write() {
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixIOThread &thread = thread_bufs_[tid];
        size_t off = 0;
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            ssize_t ret = pwrite64(thread.fd_, thread.buf_ + off, GetBlockSizeBytes(), GetOffsetBytes(tid));
            if (ret != (int) GetBlockSizeBytes()) {
                printf("Error, could not write POSIX: %s\n", strerror(errno));
                exit(1);
            }
            off += GetBlockSizeBytes();
        }
    }

    void Read() {
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixIOThread &thread = thread_bufs_[tid];
        size_t off = 0;
        int count = 0;
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            ssize_t ret = pread64(thread.fd_, thread.buf_ + off, GetBlockSizeBytes(), GetOffsetBytes(tid));
            if (ret != (int) GetBlockSizeBytes()) {
                printf("Error, could not read POSIX: %s\n", strerror(errno));
                exit(1);
            }
            count += thread.buf_[off];
            off += GetBlockSizeBytes();
        }
    }
};

}

#endif //LABSTOR_POSIX_H
