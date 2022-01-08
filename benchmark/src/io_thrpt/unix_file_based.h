//
// Created by lukemartinlogan on 1/8/22.
//

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
#include <labstor/types/thread_local.h>

namespace labstor {
class UnixFileBasedIOTest : public IOTest {
public:
    void Init(char *path, size_t block_size, size_t total_size, int ops_per_batch, int nthreads, bool do_truncate) {
        IOTest::Init(block_size, total_size, ops_per_batch, nthreads);
        //Open file & truncate
        int fd = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd < 0) {
            perror("Could not open/create file\n");
            exit(1);
        }
        if(do_truncate) {
            if(ftruncate(fd, GetTotalIO()) < 0) {
                perror("Failed to trunacate file\n");
                exit(1);
            }
        }
        close(fd);
    }
};
}

#endif //LABSTOR_FILE_BASED_H
