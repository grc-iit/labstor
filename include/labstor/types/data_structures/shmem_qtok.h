//
// Created by lukemartinlogan on 11/30/21.
//

#ifndef LABSTOR_SHMEM_QTOK_H
#define LABSTOR_SHMEM_QTOK_H

typedef uint64_t labstor_qid_t;

typedef uint32_t labstor_req_id_t;
struct labstor_qtok_t {
    labstor_qid_t qid;
    labstor_req_id_t req_id;
#ifdef __cplusplus
    labstor_qtok_t() = default;
    void Init(labstor_qid_t qid_, labstor_req_id_t req_id_) {
        qid = qid_;
        req_id = req_id_;
    }
#endif
};


#ifdef __cplusplus

#include <labstor/types/data_structures/array/shmem_array_labstor_qtok_t.h>

namespace labstor::ipc {

typedef uint64_t qid_t;
typedef labstor_qtok_t qtok_t;
typedef labstor_req_id_t req_id_t;

class qtok_set {
private:
    array_labstor_qtok_t arr_;
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
