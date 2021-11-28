//
// Created by lukemartinlogan on 11/20/21.
//

#ifndef LABSTOR_SHMEM_REQEST_MAP_H
#define LABSTOR_SHMEM_REQEST_MAP_H

#include <labstor/constants/macros.h>
#include <labstor/types/shmem_type.h>
#include "shmem_unordered_map.h"

namespace labstor::ipc {

struct request_map_bucket {
    labstor::off_t off_;

    request_map_bucket() = default;
    request_map_bucket(labstor::ipc::request *rq, void *region) {
        off_ = LABSTOR_REGION_SUB(rq, region);
    }
    request_map_bucket(const request_map_bucket &old) {
        off_ = old.off_;
    }
    inline labstor::ipc::request* GetValue(void *region) {
        if(IsMarked() || IsNull()) { return nullptr; }
        return (labstor::ipc::request*)LABSTOR_REGION_ADD(off_, region);
    }
    inline uint32_t GetKey(void *region) {
        if(off_ == 0) { return 0; }
        labstor::ipc::request *rq = (labstor::ipc::request *)LABSTOR_REGION_ADD(off_, region);
        return rq->qtok_;
    }
    inline labstor::off_t& GetAtomicValue() {
        return off_;
    }
    inline labstor::off_t& GetAtomicKey() {
        return off_;
    }
    inline static uint32_t hash(const uint32_t &qtok, void *region) {
        return qtok;
    }

    inline uint32_t IsMarked() { return GetAtomicKey() & unordered_map_atomics_null1<uint32_t>::mark; }
    inline uint32_t IsNull() { return GetAtomicKey() == unordered_map_atomics_null1<uint32_t>::null; }
    inline uint32_t GetMarkedAtomicKey() { return GetAtomicKey() | unordered_map_atomics_null1<uint32_t>::mark; }
    inline static uint32_t Null() { return unordered_map_atomics_null1<uint32_t>::null; }
};

class request_map : public unordered_map<uint32_t, labstor::ipc::request*, uint32_t, request_map_bucket> {
public:
    inline bool Set(labstor::ipc::request *rq) {
        request_map_bucket bucket(rq, region_);
        return unordered_map<uint32_t, labstor::ipc::request*, uint32_t, request_map_bucket>::Set(bucket);
    }
};

}
#endif //LABSTOR_SHMEM_REQEST_MAP_H
