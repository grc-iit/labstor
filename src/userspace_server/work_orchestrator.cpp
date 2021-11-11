//
// Created by lukemartinlogan on 9/19/21.
//

#include <labstor/ipc/request_queue.h>
#include <labstor/userspace_server/work_orchestrator.h>
#include <labstor/userspace_server/server.h>
#include <labstor/kernel_client/kernel_client.h>

#include <secure_shmem/netlink_client/shmem_user_netlink.h>
#include <workers/netlink_client/worker_user_netlink.h>

void labstor::Server::WorkOrchestrator::CreateWorkers() {
    auto labstor_config_ = scs::Singleton<labstor::Server::ConfigurationManager>::GetInstance();
    auto netlink_client__ = scs::Singleton<labstor::Kernel::NetlinkClient>::GetInstance();
    const auto &config = labstor_config_->config_["work_orchestrator"];
    int queue_depth = config["work_queue_depth"].as<int>();
    size_t time_slice_us = config["time_slice_us"].as<size_t>();
    int nworkers;

    //Server worker threads
    cpu_set_t cpus[n_cpu_];
    CPU_ZERO(cpus);
    nworkers = config["server_workers"].size();
    worker_pool_.emplace(labstor_config_->pid_, nworkers);
    auto &server_workers_ = worker_pool_[labstor_config_->pid_];
    for (const auto &worker_conf : config["server_workers"]) {
        int worker_id = worker_conf["worker_id"].as<int>();
        int cpu_id = worker_conf["cpu_id"].as<int>();
        labstor::Worker *worker = &server_workers_[worker_id];

        CPU_SET(cpu_id, cpus);
        /*worker->work_queue = malloc(queue_depth);
        worker->work_queue_depth = queue_depth;
        worker->time_slice_us = time_slice_us;*/
        worker->std_thread = std::thread(&labstor::Worker::ProcessWork, worker);
        pthread_setaffinity_np(worker->std_thread.native_handle(), n_cpu_, cpus);
        CPU_CLR(cpu_id, cpus);
    }

    //Allocate kernel worker queues
    nworkers = config["kernel_workers"].size();
    worker_pool_.emplace(0, nworkers);
    ShmemNetlinkClient shmem = ShmemNetlinkClient();
    size_t per_worker_shmem = queue_depth*sizeof(labstor::ipc::request);
    size_t shmem_size = nworkers*per_worker_shmem;
    int region_id = shmem.CreateShmem(shmem_size, false);
    shmem.GrantPidShmem(labstor_config_->pid_, region_id);
    char *region = (char*)shmem.MapShmem(region_id, shmem_size);

    //Spawn kernel workers
    WorkerNetlinkClient kernel_worker_comm_;
    kernel_worker_comm_.CreateWorkers(nworkers, region_id, shmem_size, time_slice_us);

    //Server kernel threads
    auto &kernel_workers_ = worker_pool_[labstor_config_->pid_];
    for (const auto &worker_conf : config["kernel_workers"]) {
        int worker_id = worker_conf["worker_id"].as<int>();
        int cpu_id = worker_conf["cpu_id"].as<int>();
        labstor::Worker *worker = &kernel_workers_[worker_id];

        kernel_worker_comm_.SetAffinity(worker_id, cpu_id);
        /*worker->work_queue = region + worker_id*per_worker_shmem;
        worker->work_queue_depth = queue_depth;
        worker->time_slice_us = time_slice_us;*/
    }
}
/*
void labstor::Server::WorkOrchestrator::AssignWork() {
}*/