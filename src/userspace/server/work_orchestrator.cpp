//
// Created by lukemartinlogan on 9/19/21.
//

#include <labstor/userspace/server/server.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/constants/debug.h>
#include <labstor/userspace/types/userspace_daemon.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/work_orchestrator.h>
#include <labstor/userspace/server/worker.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/util/partitioner.h>
#include <labstor/kernel/client/kernel_client.h>

#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>
#include <modules/kernel/work_orchestrator/netlink_client/work_orchestrator_client_netlink.h>

void labstor::Server::WorkOrchestrator::CreateWorkers() {
    AUTO_TRACE("labstor::Server::WorkOrchestrator::CreateWorkers")
    auto labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    auto netlink_client_ = LABSTOR_KERNEL_CLIENT;
    const auto &config = labstor_config_->config_["work_orchestrator"];
    uint32_t queue_depth = config["work_queue_depth"].as<uint32_t>();
    int nworkers;

    //Server worker threads
    nworkers = config["server_workers"].size();
    if(nworkers == 0) {
        throw WORK_ORCHESTRATOR_HAS_NO_WORKERS.format("server");
    }
    worker_pool_.emplace(pid_, nworkers);
    auto &server_workers = worker_pool_[pid_];
    for (const auto &worker_conf : config["server_workers"]) {
        int worker_id = worker_conf["worker_id"].as<int>();
        int cpu_id = worker_conf["cpu_id"].as<int>();
        TRACEPOINT("labstor::Server::WorkOrchestrator::CreateWorkers", "id", worker_id, "cpu", cpu_id)
        std::shared_ptr<labstor::UserspaceDaemon> worker_daemon = std::shared_ptr<labstor::UserspaceDaemon>(new labstor::UserspaceDaemon());
        std::shared_ptr<labstor::Server::Worker> worker = std::shared_ptr<labstor::Server::Worker>(new labstor::Server::Worker(queue_depth));
        worker_daemon->SetWorker(worker);
        worker_daemon->Start();
        worker_daemon->SetAffinity(cpu_id);
        server_workers[worker_id] = worker_daemon;
    }

    //Create kernel work queue region
    labstor::kernel::netlink::ShmemClient shmem;
    nworkers = config["kernel_workers"].size();
    if(nworkers == 0) {
        throw WORK_ORCHESTRATOR_HAS_NO_WORKERS.format("kernel");
    }
    uint32_t region_size = nworkers * labstor::ipc::work_queue::GetSize(queue_depth);
    int region_id = shmem.CreateShmem(region_size, true);
    if(region_id < 0) {
        throw WORK_ORCHESTRATOR_WORK_QUEUE_ALLOC_FAILED.format();
    }
    shmem.GrantPidShmem(getpid(), region_id);
    void *region = (char*)shmem.MapShmem(region_id, region_size);
    if(!region) {
        throw WORK_ORCHESTRATOR_WORK_QUEUE_MMAP_FAILED.format();
    }

    //Tell kernel to create work queues
    LABSTOR_KERNEL_WORK_ORCHESTRATOR_T kernel_work_orchestrator = LABSTOR_KERNEL_WORK_ORCHESTRATOR;
    kernel_work_orchestrator->CreateWorkers(nworkers, region_id, region_size, 0);

    //Worker queues
    worker_pool_.emplace(KERNEL_PID, nworkers);
    auto &kernel_workers = worker_pool_[KERNEL_PID];
    for (const auto &worker_conf : config["kernel_workers"]) {
        int worker_id = worker_conf["worker_id"].as<int>();
        int cpu_id = worker_conf["cpu_id"].as<int>();
        TRACEPOINT("labstor::Server::WorkOrchestrator::CreateKernelWorkers", "id", worker_id, "cpu", cpu_id)
        std::shared_ptr<labstor::kernel::netlink::WorkerClient> worker_daemon =
                std::shared_ptr<labstor::kernel::netlink::WorkerClient>(new labstor::kernel::netlink::WorkerClient(worker_id));
        std::shared_ptr<labstor::Server::Worker> worker =
                std::shared_ptr<labstor::Server::Worker>(new labstor::Server::Worker(queue_depth));
        worker_daemon->SetWorker(worker);
        worker_daemon->Start();
        worker_daemon->SetAffinity(cpu_id);
        kernel_workers[worker_id] = worker_daemon;
    }
}

void labstor::Server::WorkOrchestrator::AssignQueuePair(labstor::ipc::queue_pair *qp, int worker_id) {
    AUTO_TRACE("labstor::Server::WorkOrchestrator::AssignQueuePair")
    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    auto &server_workers = worker_pool_[pid_];
    if(worker_id < 0) {
        throw NOT_YET_IMPLEMENTED.format("Dynamic work orchestration");
    }
    worker_id = worker_id % server_workers.size();
    TRACEPOINT("labstor::Server::WorkOrchestrator::AssignQueuePair", worker_id)
    std::shared_ptr<labstor::Server::Worker> worker = std::dynamic_pointer_cast<labstor::Server::Worker>(server_workers[worker_id]->GetWorker());
    worker->AssignQP(qp);
}