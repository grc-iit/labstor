
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

#ifndef LABSTOR_FILE_BASED_H
#define LABSTOR_FILE_BASED_H

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
#include "labstor/types/thread_local.h"

namespace labstor {
struct UnixFileBasedIOThread {
    int fd_;
    char *buf_;
    UnixFileBasedIOThread(char *path, size_t block_size) {
        //Open file
        fd_ = open(path, O_DIRECT | O_CREAT | O_RDWR | O_SYNC | O_DSYNC, 0x644);
        if(fd_ < 0) {
            printf("Could not open/create file\n");
            exit(1);
        }
        buf_ = reinterpret_cast<char*>(aligned_alloc(4096, block_size));
        memset(buf_, 140, block_size);
    }
};
class UnixFileBasedIOTest : public IOTest {
public:
    void Init(char *path, bool do_truncate, labstor::Generator *generator) {
        IOTest::Init(generator);
        //Open file & truncate
        int fd = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd < 0) {
            perror("Could not open/create file\n");
            exit(1);
        }
        if(do_truncate) {
            if(ftruncate(fd, GetTotalIOBytes()) < 0) {
                perror("Failed to trunacate file\n");
                exit(1);
            }
        }
        close(fd);
    }
};
}

#endif //LABSTOR_FILE_BASED_H