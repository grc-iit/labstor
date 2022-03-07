//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SERVER_WORK_ORCHESTRATOR_H
#define LABSTOR_SERVER_WORK_ORCHESTRATOR_H

#include <sys/sysinfo.h>
#include <memory>
#include <unordered_map>
#include <vector>

#include <labstor/userspace/server/worker.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>

namespace labstor::Server {

class WorkOrchestrator {
private:
    int pid_;
    int n_cpu_;
    pthread_t mapper_;
    std::unordered_map<pid_t, std::vector<std::shared_ptr<labstor::Daemon>>> worker_pool_;
    std::shared_ptr<labstor::Daemon> work_balancer_;
public:
    WorkOrchestrator() {
        pid_ = getpid();
        n_cpu_ = get_nprocs_conf();
    }

    inline int GetPID() { return pid_; }
    inline int GetNumCPU() { return n_cpu_; }
    void CreateWorkers();
    void AssignQueuePair(labstor::ipc::shmem_queue_pair *qp, int worker_id=-1);
};

}

#endif //LABSTOR_SERVER_WORK_ORCHESTRATOR_H
