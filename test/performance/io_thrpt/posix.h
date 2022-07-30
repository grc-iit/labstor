
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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

#include "labstor/types/thread_local.h"

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