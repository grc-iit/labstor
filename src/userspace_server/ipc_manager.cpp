//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/util/errors.h>
#include <labstor/util/debug.h>
#include <labstor/types/basics.h>
#include <labstor/types/socket.h>
#include <labstor/userspace_server/server.h>
#include <labstor/userspace_server/ipc_manager.h>
#include <labstor/userspace_server/work_orchestrator.h>
#include <labstor/kernel_client/kernel_client.h>
#include <labstor/types/messages.h>

LABSTOR_WORK_ORCHESTRATOR_T work_orchestrator_ = LABSTOR_WORK_ORCHESTRATOR;

void labstor::Server::IPCManager::RegisterClient(int client_fd, labstor::credentials &creds) {
    AUTO_TRACE("labstor::Server::IPCManager::RegisterClient", client_fd)
    //Create new IPC
    pid_to_ipc_.Set(creds.pid, new PerProcessIPC(client_fd, creds));
    PerProcessIPC *client_ipc = pid_to_ipc_[creds.pid];

    //Create shared memory
    client_ipc->region_id_ = shmem_client_.CreateShmem(per_process_shmem_, true);
    if(client_ipc->region_id_ < 0) {
        throw SHMEM_CREATE_FAILED.format();
    }
    shmem_client_.GrantPidShmem(getpid(), client_ipc->region_id_);
    shmem_client_.GrantPidShmem(creds.pid, client_ipc->region_id_);
    client_ipc->shmem_region_ = shmem_client_.MapShmem(client_ipc->region_id_, per_process_shmem_);
    if(!client_ipc->shmem_region_) {
        throw MMAP_FAILED.format(strerror(errno));
    }
    TRACEPOINT("IPCManager: The shared memory region for PID queue pairs", (size_t)client_ipc->shmem_region_);

    //Create SHMEM allocator
    labstor::ipc::shmem_allocator *alloc = new labstor::ipc::shmem_allocator();
    alloc->Init(client_ipc->shmem_region_, per_process_shmem_, allocator_unit_);
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
    RegisterQP(client_ipc);
}

void labstor::Server::IPCManager::RegisterQP(PerProcessIPC *client_ipc) {
    AUTO_TRACE("labstor::Server::IPCManager::RegisterQP")
    //Receive SHMEM queue offsets
    labstor::ipc::register_qp_request request;
    client_ipc->GetSocket().RecvMSG((void*)&request, sizeof(labstor::ipc::register_qp_request));

    //Receive the SHMEM queue pointers
    uint32_t size = request.GetQueueArrayLength();
    labstor::ipc::queue_pair_ptr *ptrs = (labstor::ipc::queue_pair_ptr*)malloc(size);
    client_ipc->GetSocket().RecvMSG((void*)ptrs, size);
    for(int i = 0; i < request.count_; ++i) {
        labstor::ipc::queue_pair qp(ptrs[i], client_ipc->shmem_region_);
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