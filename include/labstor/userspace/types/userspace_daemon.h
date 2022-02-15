//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_USERSPACE_DAEMON_H
#define LABSTOR_USERSPACE_DAEMON_H

#include <labstor/types/daemon.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/util/timer.h>
#include <sys/sysinfo.h>
#include <sched.h>
#include <thread>
#include <future>
#include <chrono>

namespace labstor {

class UserspaceDaemon : public Daemon {
private:
    std::thread thread_;
    bool continue_work_;
    std::atomic<bool> started_;
public:
    void Start() override {
        continue_work_ = true;
        started_ = false;
        thread_ = std::thread(daemon_thread, this, worker_);
        while(!started_);
    }

    void SetStarted() {
        started_ = true;
    }

    void Pause() override {
        cpu_set_t cpus[n_cpu_];
        CPU_ZERO(cpus);
        pthread_setaffinity_np_safe(n_cpu_, cpus);
    }

    void Resume() override {
        SetAffinity(affinity_);
    }

    void Stop() override {
        continue_work_ = false;
        thread_.join();
    }

    void Wait() override {
        thread_.join();
    }

    void SetAffinity(int cpu_id) override {
        cpu_set_t cpus[n_cpu_];
        CPU_ZERO(cpus);
        CPU_SET(cpu_id, cpus);
        pthread_setaffinity_np_safe(n_cpu_, cpus);
        affinity_ = cpu_id;
    }

    bool ShouldContinue() {
        return continue_work_;
    }

private:
    static void daemon_thread(UserspaceDaemon *daemon, std::shared_ptr<DaemonWorker> worker) {
        daemon->SetStarted();
        while(daemon->ShouldContinue()) {
            worker->DoWork();
        }
    }

    inline void pthread_setaffinity_np_safe(int n_cpu, cpu_set_t *cpus) {
        int ret = pthread_setaffinity_np(thread_.native_handle(), n_cpu, cpus);
        if(ret != 0) {
            throw INVALID_AFFINITY.format(strerror(ret));
        }
    }
};

}

#endif //LABSTOR_USERSPACE_DAEMON_H
