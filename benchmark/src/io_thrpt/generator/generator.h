//
// Created by lukemartinlogan on 1/10/22.
//

#ifndef LABSTOR_GENERATOR_H
#define LABSTOR_GENERATOR_H

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace labstor {
class Generator {
private:
    std::vector<size_t> ticks_;
    size_t block_size_bytes_, block_size_units_;
    size_t batches_per_thread_, bytes_per_batch_, units_per_batch_;
    size_t total_size_bytes_;
    size_t total_ops_;
    int ops_per_batch_;
    int nthreads_;
    int unit_;
public:
    Generator() : unit_(1) {}

    void Init(size_t block_size, size_t total_size, int ops_per_batch, int nthreads) {
        nthreads_ = nthreads;
        block_size_bytes_ = block_size;
        total_size_bytes_ = total_size;
        nthreads_ = nthreads;
        batches_per_thread_ = (total_size / (block_size*ops_per_batch)) / nthreads;
        printf("TOTAL SIZE: %lu\n", total_size);
        printf("BLOCK SIZE: %lu\n", block_size);
        printf("BLOCK SIZE: %d\n", ops_per_batch);
        printf("NTHREADS: %d\n", nthreads);
        ops_per_batch_ = ops_per_batch;
        bytes_per_batch_ = ops_per_batch * block_size;
        total_ops_ = batches_per_thread_ * nthreads * ops_per_batch;
        for(int i = 0; i < GetNumThreads(); ++i) {
            ticks_.emplace_back(0);
        }
        SetOffsetUnit(1);
        ResetTicks();
    }

    int GetNumThreads() {
        return nthreads_;
    }
    size_t GetBlockSizeBytes() {
        return block_size_bytes_;
    }
    size_t GetBlockSizeUnits() {
        return block_size_units_;
    }
    size_t GetTotalIOBytes() {
        return total_size_bytes_;
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
    size_t GetBytesPerBatch() {
        return bytes_per_batch_;
    }
    size_t GetUnitsPerBatch() {
        return units_per_batch_;
    }
    size_t GetBytesPerThread() {
        return GetBatchesPerThread()*GetBytesPerBatch();
    }
    size_t GetUnitsPerThread() {
        return GetBatchesPerThread()*GetUnitsPerBatch();
    }
    void IncrementTick(int tid) {
        ++ticks_[tid];
    }
    size_t GetTick(int tid) {
        return ticks_[tid];
    }
    void ResetTicks() {
        for(auto &tick : ticks_) {
            tick = 0;
        }
    }
    void SetOffsetUnit(int unit) {
        unit_ = unit;
        block_size_units_ = block_size_bytes_ / unit;
        if((block_size_bytes_ % unit) != 0) {
            printf("Error, %lu is not a multiple of %d\n", block_size_bytes_, unit);
            exit(1);
        }
        units_per_batch_ = GetOpsPerBatch() * GetBlockSizeUnits();
    }
    int GetOffsetUnit() {
        return unit_;
    }
    virtual size_t GetOffsetBytes(int tid) = 0;
    virtual size_t GetOffsetUnits(int tid) = 0;
};
}

#endif //LABSTOR_GENERATOR_H
