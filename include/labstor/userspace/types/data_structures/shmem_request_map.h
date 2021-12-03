//
// Created by lukemartinlogan on 11/20/21.
//

#ifndef LABSTOR_SHMEM_REQEST_MAP_H
#define LABSTOR_SHMEM_REQEST_MAP_H

#include <labstor/userspace/constants/macros.h>
#include <labstor/userspace/types/shmem_type.h>
#include "shmem_unordered_map.h"

namespace labstor::ipc {

struct request_map_bucket {
    labstor::off_t off_;

    inline request_map_bucket() = default;
    inline request_map_bucket(labstor::ipc::request *rq, void *region) {
        off_ = LABSTOR_REGION_SUB(rq, region);
    }
    inline request_map_bucket(const request_map_bucket &old) {
        off_ = old.off_;
    }
    inline labstor::ipc::request* GetValue(void *region) {
        if(IsNull()) {
            TRACEPOINT("For some reason, the passed key is equal to this", off_)
            return nullptr;
        }
        return (labstor::ipc::request*)LABSTOR_REGION_ADD(off_, region);
    }
    inline uint32_t GetKey(void *region) {
        if(IsNull()) { return Null(); }
        labstor::ipc::request *rq = (labstor::ipc::request *)LABSTOR_REGION_ADD(off_, region);
        return rq->req_id_;
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

    inline bool IsMarked() { return GetAtomicKey() & unordered_map_atomics_null1<uint32_t>::mark; }
    inline bool IsNull() { return GetAtomicKey() == unordered_map_atomics_null1<uint32_t>::null; }
    inline uint32_t GetMarkedAtomicKey() { return GetAtomicKey() | unordered_map_atomics_null1<uint32_t>::mark; }
    inline static uint32_t Null() { return unordered_map_atomics_null1<uint32_t>::null; }
    inline static bool IsNullValue(labstor::ipc::request *value) { return value == nullptr; }
};

class request_map : public unordered_map<uint32_t, labstor::ipc::request*, labstor::off_t, request_map_bucket> {
public:
    inline bool Set(labstor::ipc::request *rq) {
        request_map_bucket bucket(rq, GetRegion());
        return unordered_map<uint32_t, labstor::ipc::request*, labstor::off_t, request_map_bucket>::Set(bucket);
    }
};

}
#endif //LABSTOR_SHMEM_REQEST_MAP_H
