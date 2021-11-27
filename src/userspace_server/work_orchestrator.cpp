//
// Created by lukemartinlogan on 9/19/21.
//

#include <labstor/util/errors.h>
#include <labstor/util/debug.h>
#include <labstor/types/daemon/userspace_daemon.h>
#include <labstor/types/data_structures/shmem_request_queue.h>
#include <labstor/userspace_server/macros.h>
#include <labstor/userspace_server/work_orchestrator.h>
#include <labstor/userspace_server/worker.h>
#include <labstor/userspace_server/server.h>
#include <labstor/kernel_client/kernel_client.h>

#include <modules/kernel/secure_shmem/netlink_client/shmem_user_netlink.h>
#include <modules/kernel/workers/netlink_client/worker_user_netlink.h>

void labstor::Server::WorkOrchestrator::CreateWorkers() {
    AUTO_TRACE("labstor::Server::WorkOrchestrator::CreateWorkers")
    auto labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    auto netlink_client_ = LABSTOR_KERNEL_CLIENT;
    const auto &config = labstor_config_->config_["work_orchestrator"];
    uint32_t queue_depth = config["work_queue_depth"].as<uint32_t>();
    int nworkers;

    //Server worker threads
    nworkers = config["server_workers"].size();
    worker_pool_.emplace(pid_, nworkers);
    auto &server_workers_ = worker_pool_[pid_];
    for (const auto &worker_conf : config["server_workers"]) {
        int worker_id = worker_conf["worker_id"].as<int>();
        int cpu_id = worker_conf["cpu_id"].as<int>();
        std::shared_ptr<labstor::UserspaceDaemon> worker_daemon = std::shared_ptr<labstor::UserspaceDaemon>(new labstor::UserspaceDaemon());
        std::shared_ptr<labstor::Server::Worker> worker = std::shared_ptr<labstor::Server::Worker>(new labstor::Server::Worker(queue_depth));
        worker_daemon->SetWorker(worker);
        worker_daemon->SetAffinity(cpu_id);
        server_workers_.emplace_back(worker_daemon);
    }
}

void labstor::Server::WorkOrchestrator::AssignQueuePair(labstor::ipc::queue_pair &qp, int worker_id) {
    AUTO_TRACE("labstor::Server::WorkOrchestrator::AssignQueuePair")
    auto &server_workers = worker_pool_[pid_];
    if(worker_id < 0) {
        throw NOT_YET_IMPLEMENTED.format("Dynamic work orchestration");
    }
    worker_id = worker_id % server_workers.size();
    std::shared_ptr<labstor::Server::Worker> worker = std::dynamic_pointer_cast<labstor::Server::Worker>(server_workers[worker_id]->GetWorker());
}