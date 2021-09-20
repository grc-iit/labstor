//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_WORK_ORCHESTRATOR_H
#define LABSTOR_WORK_ORCHESTRATOR_H

#include <sys/sysinfo.h>
#include <sched.h>
#include <memory>
#include <unordered_map>
#include <vector>

#include <labstor/util/singleton.h>
#include <labstor/userspace_server/worker.h>

namespace labstor {

class WorkOrchestrator {
private:
    int n_cpu_;
    pthread_t mapper_;
    std::unordered_map<pid_t, std::vector<labstor::Worker>> worker_pool_;
public:
    WorkOrchestrator() {
        n_cpu_ = get_nprocs_conf();
    }

    void CreateWorkers();
};

}

#endif //LABSTOR_WORK_ORCHESTRATOR_H
