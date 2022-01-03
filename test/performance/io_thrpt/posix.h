//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_POSIX_H
#define LABSTOR_POSIX_H

#include <sys/types.h>
#include <sys/stat.h>
#include <aio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "io_test.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <errno.h>

namespace labstor {

class PosixIO : public IOTest {
private:
    int fd_;
    size_t block_size_;
    size_t total_size_;
    void *buf_;
public:
    void Init(char *path, size_t block_size, size_t total_size) {
        fd_ = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd_ < 0) {
            printf("Could not open/create file\n");
            exit(1);
        }
        block_size_ = block_size;
        total_size_ = total_size;
        buf_ = aligned_alloc(4096, block_size_);
        memset(buf_, 0, block_size_);
    }

    void Write() {
        for (size_t i = 0; i < total_size_; i += block_size_) {
            if(write(fd_, buf_, block_size_) != block_size_) {
                printf("Error, could not write POSIX: %s\n", strerror(errno));
                exit(1);
            }
        }
    }

    void Read() {
        for (size_t i = 0; i < total_size_; i += block_size_) {
            if(read(fd_, buf_, block_size_) != block_size_) {
                printf("Error, could not read POSIX: %s\n", strerror(errno));
                exit(1);
            }
        }
    }

    size_t GetTotalIO() {
        return total_size_;
    }

    size_t GetNumOps() {
        if ((total_size_ % block_size_) == 0) {
            return total_size_ / block_size_;
        } else {
            return total_size_ / block_size_ + 1;
        }
    }
};

}

#endif //LABSTOR_POSIX_H
