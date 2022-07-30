
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