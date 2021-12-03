//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_SHMEM_QTOK_H
#define LABSTOR_SHMEM_QTOK_H

#ifdef __cplusplus

#include "shmem_array.h"

namespace labstor::ipc {

typedef uint64_t qid_t;

struct qtok_t {
    labstor::ipc::qid_t qid;
    uint32_t req_id;
};

class qtok_set {
private:
    array<qtok_t> arr_;
public:
    inline uint32_t GetLength() {
        return arr_.GetLength();
    }

    inline void Init(void *region, uint32_t region_size) {
        arr_.Init(region, region_size);
    }
    inline void Attach(void *region) {
        arr_.Attach(region);
    }

    inline void Remove(int i) {
        arr_[i].qid = -1;
        arr_[i].req_id = -1;
    }

    inline qtok_t& operator [] (int i) { return arr_[i]; }
};

}

#endif

#endif //LABSTOR_SHMEM_QTOK_H
