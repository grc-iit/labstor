//
// Created by lukemartinlogan on 1/7/22.
//

#ifndef LABSTOR_LIBAIO_H
#define LABSTOR_LIBAIO_H

#include "unix_file_based.h"
#include <libaio.h>

namespace labstor {

struct LibAIOThread {
    char *buf_;
    int fd_;
    size_t io_offset_;
    io_context_t ctx_;
    struct iocb *cbs_;
    struct io_event *events_;
    LibAIOThread(char *path, int ops_per_batch, size_t block_size, size_t io_offset) {
        //Open file
        fd_ = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
        if(fd_ < 0) {
            printf("Could not open/create file\n");
            exit(1);
        }
        buf_ = reinterpret_cast<char*>(aligned_alloc(4096, block_size));
        cbs_ = reinterpret_cast<struct iocb*>(malloc(ops_per_batch*sizeof(struct iocb)));
        events_ = reinterpret_cast<struct io_event*>(malloc(ops_per_batch*sizeof(struct io_event)));
        memset(buf_, 0, block_size);
        io_offset_ = io_offset;
        io_queue_init(ops_per_batch, &ctx_);
    }
};

class LibAIO : public UnixFileBasedIOTest {
private:
    std::vector<LibAIOThread> thread_bufs_;
public:
    void Init(char *path, size_t block_size, size_t total_size, int ops_per_batch, int nthreads, bool do_truncate) {
        UnixFileBasedIOTest::Init(path, block_size, total_size, ops_per_batch, nthreads, do_truncate);
        //Store per-thread data
        for(int i = 0; i < nthreads_; ++i) {
            thread_bufs_.emplace_back(path, GetOpsPerBatch(), block_size_, i*GetIOPerThread());
        }
    }
    void AIO(int op) {
        int tid = labstor::ThreadLocal::GetTid();
        struct LibAIOThread &thread = thread_bufs_[tid];
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            struct iocb *cb = thread.cbs_+i;
            switch(op) {
                case 0:
                    io_prep_pread(cb, thread.fd_, thread.buf_, block_size_, thread.io_offset_ + i*block_size_);
                    break;
                case 1:
                    io_prep_pwrite(cb, thread.fd_, thread.buf_, block_size_, thread.io_offset_ + i*block_size_);
                    break;
            }
            io_submit(thread.ctx_, 1, &cb);
        }
        io_getevents(thread.ctx_, GetOpsPerBatch(), GetOpsPerBatch(), thread.events_, NULL);
    }

    void Read() {
        AIO(0);
    }

    void Write() {
        AIO(1);
    }
};

}

#endif //LABSTOR_LIBAIO_H
