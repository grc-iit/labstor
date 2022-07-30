
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

#ifndef LABSTOR_SHMEM_INODE_MAP_H
#define LABSTOR_SHMEM_INODE_MAP_H

#include "labstor/types/shmem_type.h"
#include "labstor/constants/macros.h"
#include "constants.h"
#include "shmem_unordered_map.h"
#include "labstor/types/data_structures/shmem_string.h"

namespace labstor::ipc::mpmc {

struct inode_map_bucket {
    uint64_t parent_;
    labstor::off_t off_;
    uint32_t value_;

    inline void Init(std::pair<uint64_t, labstor::ipc::string> key, uint32_t value, void *region) {
        off_ = LABSTOR_REGION_SUB(key.second.GetRegion(), region);
        value_ = value;
    }
    inline uint32_t GetValue(void *region) {
        return value_;
    }
    inline std::pair<uint64_t,labstor::ipc::string> GetKey(void *region) {
        labstor::ipc::string key;
        if(IsNull()) { return std::pair(0,key); }
        key.Attach(LABSTOR_REGION_ADD(off_, region));
        return std::pair(parent_, key);
    }
    inline bool IsNull() {
        return off_ == null0_null;
    }
    static inline uint32_t KeyHash(const std::pair<uint64_t,labstor::ipc::string> key, void *region) {
        uint64_t str_hash = labstor::ipc::string::hash(key.second.c_str(), key.second.size());
        for(int i = 0; i < 5; ++i) {
            str_hash += str_hash*key.first;
            str_hash += str_hash*(key.first+1);
        }
        return str_hash;
    }
    static inline bool KeyCompare(std::pair<uint64_t,labstor::ipc::string> key1, std::pair<uint64_t,labstor::ipc::string> key2) {
        return key1 == key2;
    }
};

class inode_map : public unordered_map<std::pair<uint64_t, labstor::ipc::string>, uint32_t, inode_map_bucket> {
public:
    inline bool Set(std::pair<uint64_t,labstor::ipc::string> key, uint32_t value) {
        inode_map_bucket bucket;
        bucket.Init(key, value, GetBaseRegion());
        return unordered_map<std::pair<uint64_t, labstor::ipc::string>, uint32_t, inode_map_bucket>::Set(bucket);
    }
};

}

#endif //LABSTOR_SHMEM_INODE_MAP_H