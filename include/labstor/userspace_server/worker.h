//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_WORKER_H
#define LABSTOR_WORKER_H

#ifdef __cplusplus

#include <thread>

namespace labstor {

struct Worker {
    size_t time_slice_us;
    int worker_id;
    void *work_queue;
    size_t work_queue_depth;

    std::thread std_thread;
    void ProcessWork();
};

}

#endif

#endif //LABSTOR_WORKER_H
