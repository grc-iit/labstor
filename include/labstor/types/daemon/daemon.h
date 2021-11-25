//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_DAEMON_H
#define LABSTOR_DAEMON_H

#include <labstor/util/errors.h>
#include <sys/sysinfo.h>
#include <sched.h>
#include <thread>
#include <future>

namespace labstor {

class DaemonWorker {
public:
    virtual void DoWork() = 0;
};

class Daemon {
protected:
    int n_cpu_;
    int affinity_;
    DaemonWorker *worker_;
public:
    Daemon() {
        n_cpu_ = get_nprocs_conf();
        affinity_ = -1;
    }

    void SetWorker(DaemonWorker *worker) {
        worker_ = worker;
    }

    virtual void Start() = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Wait() = 0;
    virtual void Stop() = 0;
    virtual void SetAffinity(int cpu_id) = 0;
};

}

#endif //LABSTOR_DAEMON_H