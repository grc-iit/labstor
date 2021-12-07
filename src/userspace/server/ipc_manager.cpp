//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/util/debug.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/socket.h>
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/work_orchestrator.h>
#include <labstor/kernel/client/kernel_client.h>
#include <labstor/userspace/types/messages.h>
#include <modules/kernel/ipc_manager/netlink_client/ipc_manager_client_netlink.h>
#include <modules/kernel/work_orchestrator/netlink_client/work_orchestrator_client_netlink.h>

LABSTOR_WORK_ORCHESTRATOR_T work_orchestrator_ = LABSTOR_WORK_ORCHESTRATOR;

void labstor::Server::IPCManager::InitializeKernelIPCManager() {
    AUTO_TRACE("labstor::Server::IPCManager::CreateKernelQueues")
    uint32_t region_size = labstor_config_->config_["ipc_manager"]["kernel_shmem_mb"].as<uint32_t>() * SizeType::MB;
    uint32_t request_unit = labstor_config_->config_["ipc_manager"]["kernel_request_unit_bytes"].as<uint32_t>();

    //Create new IPC for the kernel
    pid_to_ipc_.Set(KERNEL_PID, new PerProcessIPC());
    PerProcessIPC *client_ipc = pid_to_ipc_[KERNEL_PID];

    //Create SHMEM region
    LABSTOR_KERNEL_SHMEM_ALLOC_T shmem = LABSTOR_KERNEL_SHMEM_ALLOC;
    int region_id = shmem->CreateShmem(region_size, true);
    if(region_id < 0) {
        throw WORK_ORCHESTRATOR_WORK_QUEUE_ALLOC_FAILED.format();
    }
    shmem->GrantPidShmem(getpid(), region_id);
    void *region = (char*)shmem->MapShmem(region_id, region_size);
    if(!region) {
        throw WORK_ORCHESTRATOR_WORK_QUEUE_MMAP_FAILED.format();
    }

    //Initialize allocator
    labstor::ipc::shmem_allocator *kernel_alloc;
    kernel_alloc = new labstor::ipc::shmem_allocator();
    kernel_alloc->Init(region, region, region_size, request_unit);
    client_ipc->alloc_ = kernel_alloc;
    TRACEPOINT("Kernel allocator created")

    //Create kernel IPC manager
    LABSTOR_KERNEL_IPC_MANAGER_T kernel_ipc_manager = LABSTOR_KERNEL_IPC_MANAGER;
    kernel_ipc_manager->Register(region_id);
}

void labstor::Server::IPCManager::CreateKernelQueues() {
    AUTO_TRACE("labstor::Server::IPCManager::CreateKernelQueues")
    //Load config options
    uint32_t num_queues = labstor_config_->config_["ipc_manager"]["num_kernel_queues"].as<uint32_t>();
    //uint32_t queue_size = labstor_config_->config_["ipc_manager"]["kernel_queue_size_kb"].as<uint32_t>()*SizeType::KB;
    uint32_t request_unit = labstor_config_->config_["ipc_manager"]["kernel_request_unit_bytes"].as<uint32_t>();
    uint32_t queue_size = request_unit;

    //Get Kernel IPC region
    PerProcessIPC *client_ipc = pid_to_ipc_[KERNEL_PID];

    //Allocate & register SHMEM queues for the kernel
    LABSTOR_KERNEL_WORK_ORCHESTRATOR_T kernel_work_orchestrator = LABSTOR_KERNEL_WORK_ORCHESTRATOR;
    for(int i = 0; i < num_queues; ++i) {
        //Initialize QP
        labstor::ipc::queue_pair *qp = new labstor::ipc::queue_pair();
        labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                KERNEL_PID);
        void *sq_region = client_ipc->alloc_->Alloc(queue_size);
        void *cq_region = client_ipc->alloc_->Alloc(queue_size);
        qp->Init(qid, client_ipc->alloc_->GetRegion(), sq_region, queue_size, cq_region, queue_size);
        TRACEPOINT("QP Offset",
                   (size_t)sq_region,
                   (size_t)cq_region,
                   (size_t)client_ipc->alloc_->GetRegion(),
                   LABSTOR_REGION_SUB(sq_region,client_ipc->alloc_->GetRegion()),
                   LABSTOR_REGION_SUB(cq_region,client_ipc->alloc_->GetRegion()))


        //TODO: remove
        int off = LABSTOR_REGION_SUB(sq_region,client_ipc->alloc_->GetRegion());
        if(off > client_ipc->alloc_->GetSize() || off < 0) {
            printf("Not valid region offset: %d\n", off);
            throw 1;
        }

        //Store QP internally
        if(!RegisterQueuePair(qp)) {
            throw IPC_MANAGER_CANT_REGISTER_QP.format();
        }

        //Send QP to kernel work orchestrator
        TRACEPOINT("AssignQueuePair")
        kernel_work_orchestrator->AssignQueuePair(qp, client_ipc->alloc_->GetRegion());
    }
}

void labstor::Server::IPCManager::CreatePrivateQueues() {
    AUTO_TRACE("labstor::Server::IPCManager::CreatePrivateQueues")
    //Load config options
    uint32_t region_size = labstor_config_->config_["ipc_manager"]["private_mem_mb"].as<uint32_t>()*SizeType::MB;
    uint32_t num_queues = labstor_config_->config_["ipc_manager"]["num_private_queues"].as<uint32_t>();
    //uint32_t queue_size = labstor_config_->config_["ipc_manager"]["private_queue_size_kb"].as<uint32_t>()*SizeType::KB;
    uint32_t request_unit = labstor_config_->config_["ipc_manager"]["private_request_unit_bytes"].as<uint32_t>();
    uint32_t queue_size = request_unit;

    //Allocator internal memory
    private_mem_ = malloc(region_size);

    //Initialize allocator
    labstor::ipc::shmem_allocator *private_alloc;
    private_alloc = new labstor::ipc::shmem_allocator();
    private_alloc->Init(private_mem_, private_mem_, region_size, request_unit);
    private_alloc_ = private_alloc;
    TRACEPOINT("Private allocator")

    //Allocate & register PRIVATE intermediate queues for modules to communicate internally
    labstor::ipc::queue_pair *qp;
    for(int i = 0; i < num_queues; ++i) {
        //Initialize QP
        labstor::ipc::queue_pair *qp = new labstor::ipc::queue_pair();
        labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                KERNEL_PID);
        void *sq_region = private_alloc_->Alloc(queue_size);
        void *cq_region = private_alloc_->Alloc(queue_size);
        qp->Init(qid, private_alloc_->GetRegion(), sq_region, queue_size, cq_region, queue_size);

        //Store QP internally
        if(!RegisterQueuePair(qp)) {
            throw IPC_MANAGER_CANT_REGISTER_QP.format();
        }
    }
}

void labstor::Server::IPCManager::RegisterClient(int client_fd, labstor::credentials &creds) {
    AUTO_TRACE("labstor::Server::IPCManager::RegisterClient", client_fd)
    void *region;
    
    //Create new IPC
    pid_to_ipc_.Set(creds.pid, new PerProcessIPC(client_fd, creds));
    PerProcessIPC *client_ipc = pid_to_ipc_[creds.pid];

    //Create shared memory
    LABSTOR_KERNEL_SHMEM_ALLOC_T shmem = LABSTOR_KERNEL_SHMEM_ALLOC; 
    client_ipc->region_id_ = shmem->CreateShmem(per_process_shmem_, true);
    if(client_ipc->region_id_ < 0) {
        throw SHMEM_CREATE_FAILED.format();
    }
    shmem->GrantPidShmem(getpid(), client_ipc->region_id_);
    shmem->GrantPidShmem(creds.pid, client_ipc->region_id_);
    region = shmem->MapShmem(client_ipc->region_id_, per_process_shmem_);
    if(!region) {
        throw MMAP_FAILED.format(strerror(errno));
    }
    TRACEPOINT("IPCManager: The shared memory region for PID queue pairs", (size_t)region);

    //Create SHMEM allocator
    labstor::ipc::shmem_allocator *alloc = new labstor::ipc::shmem_allocator();
    alloc->Init(region, region, per_process_shmem_, allocator_unit_);
    client_ipc->alloc_ = alloc;

    //Send shared memory to client
    labstor::ipc::setup_reply reply;
    reply.region_id = client_ipc->region_id_;
    reply.region_size = per_process_shmem_;
    reply.request_unit = allocator_unit_;
    reply.concurrency = work_orchestrator_->GetNumCPU();
    reply.queue_size = allocator_unit_;
    TRACEPOINT("Registering", reply.region_id, reply.region_size, reply.request_unit, reply.concurrency, reply.queue_size)
    client_ipc->GetSocket().SendMSG(&reply, sizeof(reply));

    //Receive and register client QPs
    RegisterClientQP(client_ipc);
}

void labstor::Server::IPCManager::RegisterClientQP(PerProcessIPC *client_ipc) {
    AUTO_TRACE("labstor::Server::IPCManager::RegisterQP")
    //Receive SHMEM queue offsets
    labstor::ipc::register_qp_request request;
    client_ipc->GetSocket().RecvMSG((void*)&request, sizeof(labstor::ipc::register_qp_request));

    //Schedule QP with the work orchestrator
    uint32_t size = request.GetQueueArrayLength();
    labstor::ipc::queue_pair_ptr *ptrs = (labstor::ipc::queue_pair_ptr*)malloc(size);
    client_ipc->GetSocket().RecvMSG((void*)ptrs, size);
    for(int i = 0; i < request.count_; ++i) {
        labstor::ipc::queue_pair *qp = new labstor::ipc::queue_pair();
        qp->Attach(ptrs[i], client_ipc->GetRegion());
        if(!RegisterQueuePair(qp)) {
            free(ptrs);
            throw IPC_MANAGER_CANT_REGISTER_QP.format();
        }
        work_orchestrator_->AssignQueuePair(qp, i);
    }
    free(ptrs);

    //Reply success
    labstor::ipc::register_qp_reply reply(0);
    client_ipc->GetSocket().SendMSG((void*)&reply, sizeof(labstor::ipc::register_qp_reply));
}

void labstor::Server::IPCManager::PauseQueues() {
    for(int pid : pids_) {
        PerProcessIPC *ipc = pid_to_ipc_[pid];
        //Search all primary streaming queues
    }
}

void labstor::Server::IPCManager::WaitForPause() {
    int num_unpaused;
    do {
        num_unpaused = 0;
    } while(num_unpaused);
}

void labstor::Server::IPCManager::ResumeQueues() {
}