//
// Created by lukemartinlogan on 1/7/22.
//

#ifndef LABSTOR_SHMEM_QTOK_SET_H
#define LABSTOR_SHMEM_QTOK_SET_H

#include <labstor/types/data_structures/mpmc/shmem_ring_buffer.h>
namespace labstor::ipc {
class qtok_set {
private:
    int off_;
    std::vector<qtok_t> qtoks_;
public:
    qtok_set() : off_(0) {}
    void Reserve(int num_qtoks) {
        qtoks_.reserve(num_qtoks);
    }
    void Enqueue(qtok_t &qtok) {
        qtoks_.emplace_back(qtok);
    }
    void Enqueue(qtok_t &&qtok) {
        qtoks_.emplace_back(qtok);
    }
    bool Dequeue(qtok_t &qtok) {
        if(off_ >= qtoks_.size()) return false;
        qtok = qtoks_[off_];
        ++off_;
        return true;
    }
};
}

#endif //LABSTOR_SHMEM_QTOK_SET_H
