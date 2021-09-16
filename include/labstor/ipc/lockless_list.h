//
// Created by lukemartinlogan on 9/15/21.
//

#ifndef LABSTOR_LOCKLESS_LIST_H
#define LABSTOR_LOCKLESS_LIST_H

namespace labstor::ipc {

struct list {
    void *region_;
    size_t region_size;
};

}

#endif //LABSTOR_LOCKLESS_LIST_H
