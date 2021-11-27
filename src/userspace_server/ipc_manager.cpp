//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/util/errors.h>
#include <labstor/util/singleton.h>
#include <labstor/types/basics.h>
#include <labstor/types/socket.h>
#include <labstor/userspace_server/server.h>
#include <labstor/userspace_server/ipc_manager.h>
#include <labstor/kernel_client/kernel_client.h>
#include <labstor/types/messages.h>

void labstor::Server::IPCManager::RegisterClient(int client_fd, labstor::credentials &creds) {
    //Create new IPC
    pid_to_ipc_.Set(creds.pid, PerProcessIPC(client_fd, creds));
    PerProcessIPC ipc = pid_to_ipc_[creds.pid];

    //Create shared memory
    ipc.region_id_ = shmem_client_.CreateShmem(per_process_shmem_, true);
    if(ipc.region_id_ < 0) {
        throw SHMEM_CREATE_FAILED.format();
    }
    shmem_client_.GrantPidShmem(getpid(), ipc.region_id_);
    ipc.shmem_region_ = shmem_client_.MapShmem(ipc.region_id_, per_process_shmem_);
    if(!ipc.shmem_region_) {
        throw MMAP_FAILED.format(strerror(errno));
    }

    //Create SHMEM allocator
    labstor::ipc::shmem_allocator *alloc = new labstor::ipc::shmem_allocator();
    alloc->Init(ipc.shmem_region_, per_process_shmem_, allocator_unit_);
    ipc.alloc_ = alloc;

    //Send shared memory to client
    labstor::ipc::setup_reply reply;
    reply.region_id = ipc.region_id_;
    reply.region_size = per_process_shmem_;
    ipc.GetSocket().SendMSG(&reply, sizeof(reply));

    //Receive and register client QPs
    RegisterQP(ipc);
}

void labstor::Server::IPCManager::RegisterQP(PerProcessIPC &client_ipc) {
    //Receive SHMEM queue offsets
    labstor::ipc::register_qp_request request;
    client_ipc.GetSocket().RecvMSG((void*)&request, sizeof(labstor::ipc::register_qp_request));

    //Receive the SHMEM queue pointers
    uint32_t size = request.count_*sizeof(labstor::ipc::queue_pair_ptr);
    labstor::ipc::queue_pair_ptr *ptrs = (labstor::ipc::queue_pair_ptr*)malloc(size);
    client_ipc.GetSocket().RecvMSG((void*)ptrs, size);
    for(int i = 0; i < request.count_; ++i) {
        labstor::ipc::queue_pair qp(ptrs[i], client_ipc.shmem_region_);
        work_orchestrator_->AssignQueuePair(qp, i);
    }
    free(ptrs);

    //Reply success
    labstor::ipc::register_qp_reply reply(0);
    client_ipc.GetSocket().SendMSG((void*)&reply, sizeof(labstor::ipc::register_qp_reply));
}

void labstor::Server::IPCManager::PauseQueues() {
    for(int pid : pids_) {
        PerProcessIPC ipc = pid_to_ipc_[pid];
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