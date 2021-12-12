//
// Created by lukemartinlogan on 12/11/21.
//

#ifndef LABSTOR_THREAD_LOCAL_H
#define LABSTOR_THREAD_LOCAL_H

#include <unistd.h>

namespace labstor {

class ThreadLocal {
    static inline bool initialized_;
    static inline int tid_;
public:
    static inline int GetTid() {
        if(!initialized_) {
            tid_ = gettid();
            initialized_ = true;
        }
        return tid_;
    }
};

}

#endif //LABSTOR_THREAD_LOCAL_H
