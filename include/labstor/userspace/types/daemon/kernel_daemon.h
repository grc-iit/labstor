//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_KERNEL_DAEMON_H
#define LABSTOR_KERNEL_DAEMON_H

#include <labstor/userspace/types/daemon/daemon.h>
#include <labstor/userspace/util/errors.h>
#include <sys/sysinfo.h>
#include <sched.h>
#include <thread>
#include <future>

namespace labstor {

class KernelDaemon : public Daemon {
private:
    int dunno;
public:
    void SetWorker(DaemonWorker *worker);
    void Start();
    void Pause();
    void Resume();
    void Stop();
    void SetAffinity(int cpu_id);
};

}

#endif //LABSTOR_KERNEL_DAEMON_H
