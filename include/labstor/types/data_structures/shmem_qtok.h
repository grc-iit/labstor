//
// Created by lukemartinlogan on 11/30/21.
//

#ifndef LABSTOR_SHMEM_QTOK_H
#define LABSTOR_SHMEM_QTOK_H


#define LABSTOR_QTOK_INVALID(qtok) (qtok.req_id==-1 || qtok.qid==-1)

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

namespace labstor::ipc {

typedef uint64_t qid_t;
typedef labstor_qtok_t qtok_t;
typedef labstor_req_id_t req_id_t;

struct qtok_set_header {
    uint32_t length_;
};

class qtok_set {
private:
    struct qtok_set_header *header_;
    qtok_t *arr_;
public:
    inline uint32_t GetLength() {
        return header_->length_;
    }
    inline void Init(void *region, uint32_t region_size) {
        header_ = (qtok_set_header*)region;
        header_->length_ = (region_size - sizeof(qtok_set_header)) / sizeof(qtok_t);
        arr_ = (qtok_t*)(header_ + 1);
    }
    inline void Attach(void *region) {
        header_ = (qtok_set_header*)region;
        arr_ = (qtok_t*)(header_ + 1);
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
