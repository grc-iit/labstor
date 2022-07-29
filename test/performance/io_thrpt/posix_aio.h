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
#include "labstor/types/thread_local.h"
#include <vector>

namespace labstor {

struct PosixAIOThread : public UnixFileBasedIOThread {
    aiocb64 *cbs_;
    struct aiocb64 **cb_list_;
    PosixAIOThread(char *path, int ops_per_batch, size_t block_size) : UnixFileBasedIOThread(path, block_size) {
        //Open file
        cbs_ = reinterpret_cast<aiocb64*>(calloc(sizeof(aiocb), ops_per_batch));
        cb_list_ = reinterpret_cast<aiocb64**>(calloc(sizeof(aiocb*), ops_per_batch));
        for(int i = 0; i < ops_per_batch; ++i) {
            cb_list_[i] = cbs_ + i;
        }
    }
};

class PosixAIO : public UnixFileBasedIOTest {
private:
    std::vector<PosixAIOThread> thread_bufs_;
public:
    PosixAIO() = default;

    void Init(char *path, bool do_truncate, labstor::Generator *generator) {
        UnixFileBasedIOTest::Init(path,  do_truncate, generator);
        //Store per-thread data
        for(int i = 0; i < GetNumThreads(); ++i) {
            thread_bufs_.emplace_back(path, GetOpsPerBatch(), GetBlockSizeBytes());
        }
    }
    void AIO(int op) {
        struct aiocb64 *cb;
        int tid = labstor::ThreadLocal::GetTid();
        struct PosixAIOThread &thread = thread_bufs_[tid];
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            cb = thread.cbs_ + i;
            cb->aio_buf = thread.buf_ + i*GetBlockSizeBytes();
            cb->aio_offset = GetOffsetBytes(tid);
            cb->aio_nbytes = GetBlockSizeBytes();
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
