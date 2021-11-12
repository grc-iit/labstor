//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/util/singleton.h>
#include <labstor/util/errors.h>
#include <labstor/types/basics.h>
#include <labstor/userspace_client/client.h>
#include <labstor/userspace_client/ipc_manager.h>
#include <secure_shmem/netlink_client/shmem_user_netlink.h>

void labstor::Client::IPCManager::Connect(int num_queues) {
    ssize_t ret;
    struct sockaddr_un client_addr;
    struct sockaddr_un server_addr;

    //Create UDP socket
    memset(&client_addr, 0x0, sizeof(struct sockaddr_un));
    client_addr.sun_family = AF_UNIX;
    serverfd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverfd_ == -1) {
        throw UNIX_SOCKET_FAILED.format(strerror(errno));
    }
    if (bind(serverfd_, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_un)) == -1) {
        close(serverfd_);
        throw UNIX_BIND_FAILED.format(strerror(errno));
    }

    //Set server address
    memset(&server_addr, 0x0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, TRUSTED_SERVER_PATH, sizeof(server_addr.sun_path)-1);
    if (connect(serverfd_, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un)) == -1) {
        close(serverfd_);
        throw UNIX_CONNECT_FAILED.format(strerror(errno));
    }

    //Create the SHMEM queues
    CreateQueuesSHMEM(num_queues);
    CreateInternalQueues(num_queues);
}

void labstor::Client::IPCManager::SendMSG(void *buffer, size_t size) {
    int ret = send(serverfd_, buffer, size, 0);
    if (ret == -1) {
        throw UNIX_SENDMSG_FAILED.format(strerror(errno));
    }
}

void labstor::Client::IPCManager::RecvMSG(void *buffer, size_t size) {
    int ret = recv(serverfd_, buffer, size, 0);
    if (ret == -1) {
        throw UNIX_RECVMSG_FAILED.format(strerror(errno));
    }
}

void labstor::Client::IPCManager::CreateQueuesSHMEM(int num_queues) {
    labstor::shmem_allocator *shmem_alloc;

    //Send an IPC request to the server
    labstor::setup_request request;
    request.num_queues = num_queues;
    SendMSG((void*)&request, sizeof(labstor::setup_request));

    //Receive SHMEM IPCs
    labstor::setup_reply reply;
    RecvMSG((void*)&reply, sizeof(labstor::setup_reply));

    //Create SHMEM allocator
    void *region = ShmemNetlinkClient::MapShmem(reply.region_id, reply.region_size);
    shmem_alloc = new shmem_allocator();
    shmem_alloc->Init(region, reply.region_size, reply.request_unit, reply.concurrency);
    shmem_alloc_ = shmem_alloc;

    //Create SHMEM request queues
    for(int i = 0; i < reply.num_queues; ++i) {
        void *sq_region = shmem_alloc_->Alloc(reply.queue_size, sched_getcpu());
        void *cq_region = shmem_alloc_->Alloc(reply.queue_size, sched_getcpu());

    }
}

void labstor::Client::IPCManager::CreateInternalQueues(int num_queues) {
    labstor::shmem_allocator *internal_alloc;
    uint32_t region_size = labstor::ipc::request_queue::MinimumRegionSize()*num_queues*3;
    void *region = malloc(region_size);

    int request_unit = 256;
    int concurrency = 8;
    internal_alloc = new labstor::shmem_allocator();
    internal_alloc->Init(region, region_size, request_unit, concurrency);
    internal_alloc_ = internal_alloc;

    for(int i = 0; i < num_queues; ++i) {
        void *sq_region = internal_alloc_->Alloc(request_unit, sched_getcpu());
        void *cq_region = internal_alloc_->Alloc(request_unit, sched_getcpu());
    }
}

void labstor::Client::IPCManager::PauseQueues() {
}

void labstor::Client::IPCManager::ResumeQueues() {
}