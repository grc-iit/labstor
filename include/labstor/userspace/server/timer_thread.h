//
// Created by lukemartinlogan on 5/15/22.
//

#ifndef LABSTOR_TIMER_THREAD_H
#define LABSTOR_TIMER_THREAD_H

#include <labstor/userspace/util/errors.h>
#include <labstor/types/daemon.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/types/messages.h>

namespace labstor::Server {

class TimerWorker : public DaemonWorker {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    TimerWorker() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    void DoWork() override {

    }
};

}

#endif //LABSTOR_TIMER_THREAD_H
