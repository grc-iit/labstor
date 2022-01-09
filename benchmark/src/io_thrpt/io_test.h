//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_IO_TEST_H
#define LABSTOR_IO_TEST_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace labstor {

class IOTest {
protected:
    size_t block_size_;
    size_t total_size_;
    size_t total_ops_;
    size_t batches_per_thread_;
    size_t io_per_batch_;
    int ops_per_batch_;
    int nthreads_;

public:
    virtual void Read() = 0;
    virtual void Write() = 0;

    void Init(size_t block_size, size_t total_size, int ops_per_batch, int nthreads) {
        block_size_ = block_size;
        total_size_ = total_size;
        nthreads_ = nthreads;
        batches_per_thread_ = (total_size / (block_size*ops_per_batch)) / nthreads;
        ops_per_batch_ = ops_per_batch;
        io_per_batch_ = ops_per_batch * block_size;
        total_ops_ = batches_per_thread_ * nthreads * ops_per_batch * block_size;
        if(batches_per_thread_ == 0) {
            printf("More threads than I/O operations\n");
            exit(1);
        }
    }
    size_t GetBlockSize() {
        return block_size_;
    }
    int GetNumThreads() {
        return nthreads_;
    }
    size_t GetTotalIO() {
        return total_size_;
    }
    size_t GetTotalNumOps() {
        return total_ops_;
    }
    size_t GetBatchesPerThread() {
        return batches_per_thread_;
    }
    size_t GetOpsPerBatch() {
        return ops_per_batch_;
    }
    size_t GetIOPerBatch() {
        return io_per_batch_;
    }
    size_t GetIOPerThread() {
        return GetBatchesPerThread()*GetIOPerBatch();
    }
};

}

#endif //LABSTOR_IO_TEST_H
