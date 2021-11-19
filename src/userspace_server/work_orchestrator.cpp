//
// Created by lukemartinlogan on 9/19/21.
//

#include <labstor/types/data_structures/shmem_request_queue.h>
#include <labstor/userspace_server/macros.h>
#include <labstor/userspace_server/work_orchestrator.h>
#include <labstor/userspace_server/server.h>
#include <labstor/kernel_client/kernel_client.h>

#include <secure_shmem/netlink_client/shmem_user_netlink.h>
#include <workers/netlink_client/worker_user_netlink.h>

void labstor::Server::WorkOrchestrator::CreateWorkers() {
    auto labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    auto netlink_client_ = LABSTOR_KERNEL_CLIENT;
    const auto &config = labstor_config_->config_["work_orchestrator"];
    //int queue_depth = config["work_queue_depth"].as<int>();
    //size_t time_slice_us = config["time_slice_us"].as<size_t>();
    //int nworkers;

    //Server worker threads
    /*cpu_set_t cpus[n_cpu_];
    CPU_ZERO(cpus);
    nworkers = config["server_workers"].size();
    worker_pool_.emplace(labstor_config_->pid_, nworkers);
    auto &server_workers_ = worker_pool_[labstor_config_->pid_];
    for (const auto &worker_conf : config["server_workers"]) {
        int worker_id = worker_conf["worker_id"].as<int>();
        int cpu_id = worker_conf["cpu_id"].as<int>();
        labstor::Worker *worker = &server_workers_[worker_id];
    }*/
}
/*
void labstor::Server::WorkOrchestrator::AssignWork() {
}*/