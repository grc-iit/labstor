//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SERVER_WORKER_H
#define LABSTOR_SERVER_WORKER_H

#ifdef __cplusplus

#include <thread>

namespace labstor {

class Worker {
private:
    size_t time_slice_us;
    int worker_id;
    void *work_queue;
    size_t work_queue_depth;
public:
    std::thread std_thread;
    void ProcessWork();
};

}

#endif

#ifndef __cplusplus
#endif

#endif //LABSTOR_SERVER_WORKER_H
