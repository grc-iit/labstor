
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

#ifndef LABSTOR_IO_TEST_H
#define LABSTOR_IO_TEST_H

#include "test/performance/io_thrpt/generator/generator.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include "io_logger.h"
#include "labstor/userspace/util/timer.h"

namespace labstor {
class IOTest {
private:
    labstor::Generator *generator_;
    labstor::ThreadedHighResMonotonicTimer timer_;
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
    size_t GetTotalNumOps() {
        return generator_->GetTotalNumOps();
    }
    int GetNumThreads() {
        return generator_->GetNumThreads();
    }
    size_t GetOpsPerBatch() {
        return generator_->GetOpsPerBatch();
    }
    size_t GetBatchesPerThread() {
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
    labstor::ThreadedHighResMonotonicTimer& GetTimer() {
        return timer_;
    }
};

}

#endif //LABSTOR_IO_TEST_H