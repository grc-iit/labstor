//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_IO_TEST_H
#define LABSTOR_IO_TEST_H

#include "generator/generator.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace labstor {
class IOTest {
private:
    labstor::Generator *generator_;
public:
    void Init(labstor::Generator *generator) {
        generator_ = generator;
    }
    labstor::Generator* GetGenerator() {
        return generator_;
    }
    virtual void Read() = 0;
    virtual void Write() = 0;

    size_t GetTotalIOBytes() {
        return generator_->GetTotalIOBytes();
    }
    int GetTotalNumOps() {
        return generator_->GetTotalNumOps();
    }
    int GetNumThreads() {
        return generator_->GetNumThreads();
    }
    size_t GetOpsPerBatch() {
        return generator_->GetOpsPerBatch();
    }
    int GetBatchesPerThread() {
        return generator_->GetBatchesPerThread();
    }
    size_t GetBlockSizeBytes() {
        return generator_->GetBlockSizeBytes();
    }
    size_t GetBlockSizeUnits() {
        return generator_->GetBlockSizeUnits();
    }
    size_t GetOffsetBytes(int tid) {
        return generator_->GetOffsetBytes(tid);
    }
    size_t GetOffsetUnits(int tid) {
        return generator_->GetOffsetUnits(tid);
    }
};

}

#endif //LABSTOR_IO_TEST_H
