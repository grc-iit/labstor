//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_LIST_H
#define LABSTOR_LIST_H

namespace labstor::ipc{

template<typename T>
struct list_entry {
    struct list_entry *next;
    T *data;
};

template<typename T>
struct list {
    struct list_entry<T> *head, *tail;
    list() : head(nullptr), tail(nullptr) {}

    void append(T *data) {
        if(head == nullptr && tail == nullptr) {
            head =
        }
    }
};

}
#endif //LABSTOR_LIST_H
