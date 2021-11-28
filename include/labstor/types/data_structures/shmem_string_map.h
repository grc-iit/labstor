//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_string_map_H
#define LABSTOR_string_map_H

#include <atomic>
#include "shmem_string.h"

#ifdef __cplusplus
#include "labstor/types/shmem_type.h"
#include "shmem_unordered_map.h"

namespace labstor::ipc {

struct string_map_bucket {
    labstor::off_t off_;
    uint32_t value_;

    string_map_bucket() = default;
    string_map_bucket(labstor::ipc::string key, uint32_t value, void *region) {
        off_ = LABSTOR_REGION_SUB(key.GetRegion(), region);
        value_ = value;
    }
    inline uint32_t GetValue(void *region) {
        return value_;
    }
    inline labstor::ipc::string GetKey(void *region) {
        labstor::ipc::string key;
        if(off_ == 0) { return key; }
        key.Attach(LABSTOR_REGION_ADD(off_, region));
        return key;
    }
    inline uint32_t& GetAtomicValue() {
        return value_;
    }
    inline labstor::off_t& GetAtomicKey() {
        return off_;
    }
    inline static uint32_t hash(const labstor::ipc::string &key, void *region) {
        return labstor::ipc::string::hash(key.c_str(), key.size());
    }

    inline uint32_t IsMarked() { return GetAtomicKey() & unordered_map_atomics_null0<uint32_t>::mark; }
    inline uint32_t IsNull() { return GetAtomicKey() == unordered_map_atomics_null0<uint32_t>::null; }
    inline uint32_t GetMarkedAtomicKey() { return GetAtomicKey() | unordered_map_atomics_null0<uint32_t>::mark; }
    inline static uint32_t Null() { return unordered_map_atomics_null0<uint32_t>::null; }
};

class string_map : public unordered_map<labstor::ipc::string, uint32_t, uint32_t, string_map_bucket> {
public:
    inline bool Set(labstor::ipc::string key, uint32_t value) {
        string_map_bucket bucket(key, value, region_);
        return unordered_map<labstor::ipc::string, uint32_t, uint32_t, string_map_bucket>::Set(bucket);
    }
};

}

#endif

#endif //LABSTOR_string_map_H
