//
// Created by lukemartinlogan on 1/10/22.
//

#ifndef LABSTOR_SEQUENTIAL_H
#define LABSTOR_SEQUENTIAL_H

#include "generator.h"
#include <cstdlib>
#include <cstdint>
#include <cstdio>

namespace labstor {

class SequentialGenerator : public Generator {
public:
    void Init(size_t block_size, size_t total_size, int ops_per_batch, int nthreads) {
        Generator::Init(block_size, total_size, ops_per_batch, nthreads);
    }
    size_t GetOffsetBytes(int tid) {
        size_t off = GetBytesPerThread()*tid + GetTick(tid)*GetBlockSizeBytes();
        IncrementTick(tid);
        return off;
    }
    size_t GetOffsetUnits(int tid) {
        size_t off = GetUnitsPerThread()*tid + GetTick(tid)*GetBlockSizeUnits();
        IncrementTick(tid);
        return off;
    }
};

}

#endif //LABSTOR_SEQUENTIAL_H
