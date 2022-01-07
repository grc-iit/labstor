//
// Created by lukemartinlogan on 1/7/22.
//

#ifndef LABSTOR_LIBAIO_H
#define LABSTOR_LIBAIO_H

namespace labstor {

struct LibAIOThread {
    char *buf_;
    int fd_;
    size_t io_offset_;
    LibAIOThread(char *path, int ops_per_thread, size_t block_size, size_t io_offset) {
        //Open file
        fd_ = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd_ < 0) {
            printf("Could not open/create file\n");
            exit(1);
        }
        buf_ = reinterpret_cast<char*>(aligned_alloc(4096, block_size));
        memset(buf_, 0, block_size);
        io_offset_ = io_offset;
    }
};

class LibAIO : public IOTest {
private:
    std::vector<PosixAIOThread> thread_bufs_;
public:
    void Init(char *path, size_t block_size, size_t total_size, int ops_per_batch, int nthreads, bool do_truncate) {
        IOTest::Init(block_size, total_size, ops_per_batch, nthreads);
    }
    void AIO(int op) {
    }

    void Read() {
        AIO(LIO_READ);
    }

    void Write() {
        AIO(LIO_WRITE);
    }
};

}

#endif //LABSTOR_LIBAIO_H
