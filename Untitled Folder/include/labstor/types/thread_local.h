//
// Created by lukemartinlogan on 12/11/21.
//

#ifndef LABSTOR_THREAD_LOCAL_H
#define LABSTOR_THREAD_LOCAL_H

#include <labstor/types/basics.h>
#include <unistd.h>

namespace labstor {

extern uint32_t thread_local_counter_;
extern thread_local uint32_t thread_local_initialized_;
extern thread_local uint32_t thread_local_tid_;

class ThreadLocal {
public:
    static inline int GetTid() {
        if(!thread_local_initialized_) {
            thread_local_tid_ = __atomic_fetch_add(&thread_local_counter_,1,__ATOMIC_RELAXED);
            thread_local_initialized_ = true;
        }
        return thread_local_tid_;
    }
};

}

#endif //LABSTOR_THREAD_LOCAL_H
