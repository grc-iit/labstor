//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/userspace/server/server.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/constants/debug.h>
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
    AUTO_TRACE("labstor::Server::IPCManager::InitializeKernelIPCManager")
    uint32_t region_size = labstor_config_->config_["ipc_manager"]["kernel_shmem_mb"].as<uint32_t>() * SizeType::MB;
    uint32_t request_unit = labstor_config_->config_["ipc_manager"]["kernel_request_unit_bytes"].as<uint32_t>();
    uint32_t queue_depth = labstor_config_->config_["ipc_manager"]["kernel_queue_depth"].as<uint32_t>();
    uint32_t num_kernel_queues = labstor_config_->config_["ipc_manager"]["num_kernel_queues"].as<uint32_t>();
    uint32_t queue_region_size = num_kernel_queues * labstor::ipc::queue_pair::GetSize(queue_depth);
    uint32_t request_region_size = region_size - queue_region_size;

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

    //Initialize request allocator (returns userspace addresses)
    labstor::ipc::shmem_allocator *kernel_alloc;
    kernel_alloc = new labstor::ipc::shmem_allocator();
    kernel_alloc->Init(region, region, request_region_size, request_unit);
    client_ipc->alloc_ = kernel_alloc;
    TRACEPOINT("Kernel request allocator created")

    //Create kernel IPC manager
    LABSTOR_KERNEL_IPC_MANAGER_T kernel_ipc_manager = LABSTOR_KERNEL_IPC_MANAGER;
    kern_base_region_ = kernel_ipc_manager->Register(region_id);

    //Initialize kernel queue allocator (returns userspace addresses)
    TRACEPOINT("Kernel queue allocator created", (size_t)kern_base_region_)
    client_ipc->qp_alloc_ = new labstor::segment_allocator();
    client_ipc->qp_alloc_->Init(
            LABSTOR_REGION_ADD(request_region_size, client_ipc->GetRegion()), //Back of the SHMEM region
            queue_region_size
    );

    //Initialize kernel queue allocator (returns kernel-space addresses)
    TRACEPOINT("Kernel queue allocator created")
    kern_qp_alloc_ = new labstor::segment_allocator();
    kern_qp_alloc_->Init(
            LABSTOR_REGION_ADD(request_region_size, kern_base_region_), //Back of the SHMEM region
            queue_region_size
    );
}

void labstor::Server::IPCManager::CreateKernelQueues() {
    AUTO_TRACE("labstor::Server::IPCManager::CreateKernelQueues")
    //Load config options
    uint32_t queue_depth = labstor_config_->config_["ipc_manager"]["kernel_queue_depth"].as<uint32_t>();
    uint32_t num_kernel_queues = labstor_config_->config_["ipc_manager"]["num_kernel_queues"].as<uint32_t>();
    uint32_t request_queue_size = labstor::ipc::request_queue::GetSize(queue_depth);
    uint32_t request_map_size = labstor::ipc::request_map::GetSize(queue_depth);
    uint32_t queue_size = labstor::ipc::queue_pair::GetSize(queue_depth);
    labstor::ipc::queue_pair_ptr ptr;

    TRACEPOINT("labstor::Server::IPCManager::CreateKernelQueues", request_queue_size, request_map_size, queue_size)

    //Get Kernel IPC region
    PerProcessIPC *client_ipc = pid_to_ipc_[KERNEL_PID];

    //Allocate & register SHMEM queues for the kernel
    LABSTOR_KERNEL_WORK_ORCHESTRATOR_T kernel_work_orchestrator = LABSTOR_KERNEL_WORK_ORCHESTRATOR;
    for(int i = 0; i < num_kernel_queues; ++i) {
        //Initialize kernel QP (userspace address)
        labstor::ipc::queue_pair *remote_qp = client_ipc->qp_alloc_->Alloc<labstor::ipc::queue_pair>(sizeof(labstor::ipc::queue_pair));
        labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_kernel_queues,
                KERNEL_PID);
        void *sq_region = client_ipc->qp_alloc_->Alloc(request_queue_size);
        void *cq_region = client_ipc->qp_alloc_->Alloc(request_map_size);
        remote_qp->Init(qid, client_ipc->alloc_->GetRegion(), sq_region, request_queue_size, cq_region, request_map_size);
        TRACEPOINT("labstor::Server::IPCManager::CreateKernelQueues", "qid", remote_qp->GetQid(), "depth", remote_qp->GetDepth(),
                   "offset2", LABSTOR_REGION_SUB(remote_qp->cq.GetRegion(), client_ipc->GetRegion()));
        remote_qp->GetPointer(ptr, client_ipc->alloc_->GetRegion());

        //Make a new QP that holds userspace addresses & store internally
        labstor::ipc::queue_pair *qp = new labstor::ipc::queue_pair();
        qp->Attach(ptr, client_ipc->alloc_->GetRegion());
        if(!RegisterQueuePair(qp)) {
            throw IPC_MANAGER_CANT_REGISTER_QP.format();
        }
        TRACEPOINT("RemoteAttach3", (size_t)remote_qp->sq.base_region_, (size_t)remote_qp->sq.header_);

        //Enqeueue kernel address in kernel work orchestrator
        labstor::ipc::queue_pair *qp_kern = kern_qp_alloc_->Alloc<labstor::ipc::queue_pair>(queue_size);
        TRACEPOINT("AssignQueuePair", (size_t)qp_kern);
        kernel_work_orchestrator->AssignQueuePair(qp, qp_kern, ptr);
    }
}

void labstor::Server::IPCManager::CreatePrivateQueues() {
    AUTO_TRACE("labstor::Server::IPCManager::CreatePrivateQueues")
    //Load config options
    uint32_t region_size = labstor_config_->config_["ipc_manager"]["private_mem_mb"].as<uint32_t>()*SizeType::MB;
    uint32_t request_unit = labstor_config_->config_["ipc_manager"]["private_request_unit_bytes"].as<uint32_t>();
    uint32_t queue_depth = labstor_config_->config_["ipc_manager"]["private_queue_depth"].as<uint32_t>();
    uint32_t num_private_queues = labstor_config_->config_["ipc_manager"]["num_private_queues"].as<uint32_t>();
    uint32_t queue_region_size = num_private_queues * labstor::ipc::queue_pair::GetSize(queue_depth);
    uint32_t request_region_size = region_size - queue_region_size;
    uint32_t request_queue_size = labstor::ipc::request_queue::GetSize(queue_depth);
    uint32_t request_map_size = labstor::ipc::request_map::GetSize(queue_depth);

    //Create new IPC for private queues
    pid_to_ipc_.Set(pid_, new PerProcessIPC());
    PerProcessIPC *client_ipc = pid_to_ipc_[pid_];

    //Allocator internal memory
    private_mem_ = malloc(region_size);

    //Initialize request allocator
    labstor::ipc::shmem_allocator *private_alloc;
    private_alloc = new labstor::ipc::shmem_allocator();
    private_alloc->Init(private_mem_, private_mem_, request_region_size, request_unit);
    client_ipc->alloc_ = private_alloc;
    private_alloc_ = private_alloc;
    TRACEPOINT("Private allocator", (size_t)private_alloc_)

    //Initialize queue allocator
    client_ipc->qp_alloc_ = new labstor::segment_allocator();
    client_ipc->qp_alloc_->Init(LABSTOR_REGION_ADD(request_region_size, client_ipc->GetRegion()),
                                queue_region_size);

    //Allocate & register PRIVATE intermediate streaming queues for modules to communicate internally
    labstor::ipc::queue_pair *qp;
    for(int i = 0; i < num_private_queues; ++i) {
        //Initialize QP
        labstor::ipc::queue_pair *qp = client_ipc->qp_alloc_->Alloc<labstor::ipc::queue_pair>(sizeof(labstor::ipc::queue_pair));
        labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_private_queues,
                pid_);
        void *sq_region = client_ipc->qp_alloc_->Alloc(request_queue_size);
        void *cq_region = client_ipc->qp_alloc_->Alloc(request_map_size);
        qp->Init(qid, private_alloc_->GetRegion(), sq_region, request_queue_size, cq_region, request_map_size);
        TRACEPOINT("labstor::Server::IPCManager::CreatePrivateQueues", "qid", qp->GetQid(), "depth", qp->GetDepth(),
                   "offset2", LABSTOR_REGION_SUB(qp->cq.GetRegion(), client_ipc->GetRegion()))

        //Store QP internally
        if(!RegisterQueuePair(qp)) {
            throw IPC_MANAGER_CANT_REGISTER_QP.format();
        }

        //Schedule queue pair
        work_orchestrator_->AssignQueuePair(qp, i);
    }
}

void labstor::Server::IPCManager::RegisterClient(int client_fd, labstor::credentials &creds) {
    AUTO_TRACE("labstor::Server::IPCManager::RegisterClient", client_fd)
    void *region;
    uint32_t region_size = labstor_config_->config_["ipc_manager"]["client_shmem_kb"].as<uint32_t>()*SizeType::KB;
    uint32_t request_unit = labstor_config_->config_["ipc_manager"]["client_request_unit_bytes"].as<uint32_t>();
    uint32_t queue_depth = labstor_config_->config_["ipc_manager"]["client_queue_depth"].as<uint32_t>();
    uint32_t num_client_queues = labstor_config_->config_["ipc_manager"]["num_client_queues"].as<uint32_t>();
    uint32_t queue_region_size = num_client_queues * labstor::ipc::queue_pair::GetSize(queue_depth);
    uint32_t request_region_size = region_size - queue_region_size;

    //Create new IPC
    pid_to_ipc_.Set(creds.pid, new PerProcessIPC(client_fd, creds));
    PerProcessIPC *client_ipc = pid_to_ipc_[creds.pid];

    //Create shared memory
    LABSTOR_KERNEL_SHMEM_ALLOC_T shmem = LABSTOR_KERNEL_SHMEM_ALLOC; 
    client_ipc->region_id_ = shmem->CreateShmem(region_size, true);
    if(client_ipc->region_id_ < 0) {
        throw SHMEM_CREATE_FAILED.format();
    }
    shmem->GrantPidShmem(getpid(), client_ipc->region_id_);
    shmem->GrantPidShmem(creds.pid, client_ipc->region_id_);
    region = shmem->MapShmem(client_ipc->region_id_, region_size);
    if(!region) {
        throw MMAP_FAILED.format(strerror(errno));
    }

    //Send shared memory to client
    labstor::ipc::setup_reply reply;
    reply.region_id = client_ipc->region_id_;
    reply.region_size = region_size;
    reply.request_unit = request_unit;
    reply.request_region_size = request_region_size;
    reply.queue_region_size = queue_region_size;
    reply.queue_depth = queue_depth;
    TRACEPOINT("Registering", reply.region_id, reply.region_size, reply.request_unit)
    client_ipc->GetSocket().SendMSG(&reply, sizeof(reply));

    //Receive and register client QPs
    RegisterClientQP(client_ipc, region);
}

void labstor::Server::IPCManager::RegisterClientQP(PerProcessIPC *client_ipc, void *region) {
    AUTO_TRACE("labstor::Server::IPCManager::RegisterQP")
    //Receive SHMEM queue offsets
    labstor::ipc::register_qp_request request;
    client_ipc->GetSocket().RecvMSG((void*)&request, sizeof(labstor::ipc::register_qp_request));
    uint32_t size = request.GetQueueArrayLength();
    labstor::ipc::queue_pair_ptr *ptrs = (labstor::ipc::queue_pair_ptr*)malloc(size);
    client_ipc->GetSocket().RecvMSG((void*)ptrs, size);

    //Attach request allocator
    labstor::ipc::shmem_allocator *alloc = new labstor::ipc::shmem_allocator();
    alloc->Attach(region, region);
    client_ipc->alloc_ = alloc;

    //Schedule QP with the work orchestrator
    for(int i = 0; i < request.count_; ++i) {
        labstor::ipc::queue_pair *qp = new labstor::ipc::queue_pair();
        qp->Attach(ptrs[i], client_ipc->GetRegion());
        TRACEPOINT("labstor::Server::IPCManager::RegisterClientQP",
                   "qid", qp->GetQid(), "depth", qp->GetDepth(),
                   "offset2", LABSTOR_REGION_SUB(qp->cq.GetRegion(), client_ipc->GetRegion()))
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