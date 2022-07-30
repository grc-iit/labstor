
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

#ifndef LABSTOR_INT_MAP_H
#define LABSTOR_INT_MAP_H

#include "constants.h"
#include "shmem_unordered_map.h"
#include "labstor/userspace/util/errors.h"
#include "labstor/types/shmem_type.h"

namespace labstor::ipc::mpmc {

template<typename S, typename T>
struct int_map_bucket {
    S key_;
    T value_;
    inline void Init(S key, T value) {
        key_ = key;
        value_ = value;
    }
    inline T GetValue(void *region) {
        return value_;
    }
    inline S GetKey(void *region) {
        return key_;
    }
    static inline uint32_t KeyHash(const S key, const void *region) {
        return key;
    }
    static inline bool KeyCompare(S key1, S key2) {
        return key1==key2;
    }
};

template<typename S, typename T>
class int_map : public unordered_map<S,T,int_map_bucket<S,T>> {
public:
    inline bool Set(S key, T value) {
        int_map_bucket<S,T> bucket;
        bucket.Init(key, value);
        return unordered_map<S,T,int_map_bucket<S,T>>::Set(bucket);
    }
};

}

#endif //LABSTOR_INT_MAP_H