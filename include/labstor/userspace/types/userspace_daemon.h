//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_USERSPACE_DAEMON_H
#define LABSTOR_USERSPACE_DAEMON_H

#include <labstor/types/daemon.h>
#include <labstor/userspace/util/errors.h>
#include <sys/sysinfo.h>
#include <sched.h>
#include <thread>
#include <future>
#include <chrono>

namespace labstor {

class UserspaceDaemon : public Daemon {
private:
    std::thread thread_;
    std::promise<bool> promise_;
    std::future<bool> future_;
public:
    void Start() override {
        std::promise<bool> is_started;
        std::future<bool> is_started_future = is_started.get_future();
        future_ = promise_.get_future();
        thread_ = std::thread(daemon_thread, worker_, std::ref(is_started), std::ref(future_));
        while(is_started_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {}
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
        promise_.set_value(true);
        thread_.join();
    }

    void Wait() override {
        thread_.join();
    }

    void SetAffinity(int cpu_id) override {
        int ret = 0;
        cpu_set_t cpus[n_cpu_];
        CPU_ZERO(cpus);
        CPU_SET(cpu_id, cpus);
        pthread_setaffinity_np_safe(n_cpu_, cpus);
        affinity_ = cpu_id;
    }

private:
    static void daemon_thread(std::shared_ptr<DaemonWorker> worker, std::promise<bool>& is_started, std::future<bool>& future) {
        is_started.set_value(true);
        while(future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
            worker->DoWork();
        }
    }

    inline void pthread_setaffinity_np_safe(int n_cpu, cpu_set_t *cpus) {
        int ret = pthread_setaffinity_np(thread_.native_handle(), n_cpu, cpus);
        if(ret != 0) {
            throw INVALID_AFFINITY.format(strerror(errno));
        }
    }
};

}

#endif //LABSTOR_USERSPACE_DAEMON_H
