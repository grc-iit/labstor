//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/types/messages.h>
#include <labstor/util/singleton.h>
#include <labstor/util/errors.h>
#include <labstor/types/basics.h>
#include <labstor/types/socket.h>
#include <labstor/userspace_client/client.h>
#include <labstor/userspace_client/ipc_manager.h>
#include <secure_shmem/netlink_client/shmem_user_netlink.h>

void labstor::Client::IPCManager::Connect(int num_queues) {
    ssize_t ret;
    int serverfd;
    struct sockaddr_un client_addr;
    struct sockaddr_un server_addr;

    //Create UDP socket
    memset(&client_addr, 0x0, sizeof(struct sockaddr_un));
    client_addr.sun_family = AF_UNIX;
    serverfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverfd == -1) {
        throw UNIX_SOCKET_FAILED.format(strerror(errno));
    }
    if (bind(serverfd, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_un)) == -1) {
        close(serverfd);
        throw UNIX_BIND_FAILED.format(strerror(errno));
    }

    //Set server address & connect
    memset(&server_addr, 0x0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, TRUSTED_SERVER_PATH, sizeof(server_addr.sun_path)-1);
    if (connect(serverfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un)) == -1) {
        close(serverfd);
        throw UNIX_CONNECT_FAILED.format(strerror(errno));
    }
    serversock_.SetFd(serverfd);

    //Receive SHMEM region
    labstor::setup_reply reply;
    serversock_.RecvMSG(&reply, sizeof(reply));

    //Attach SHMEM allocator
    /*labstor::ipc::shmem_allocator *shmem_alloc;
    void *region = ShmemNetlinkClient::MapShmem(reply.region_id, reply.region_size);
    shmem_alloc = new shmem_allocator();
    shmem_alloc->Init(region, reply.region_size, reply.request_unit, reply.concurrency);
    shmem_alloc_ = shmem_alloc;*/

    //Create the SHMEM queues
    CreateQueuesSHMEM(num_queues);
}

void labstor::Client::IPCManager::CreateQueuesSHMEM(int num_queues) {
    //Allocate SHMEM queues for the client (one per-core)
    /*
    for(int i = 0; i < reply.num_queues; ++i) {
        void *sq_region = shmem_alloc_->Alloc(reply.queue_size, sched_getcpu());
        void *cq_region = shmem_alloc_->Alloc(reply.queue_size, sched_getcpu());
    }*/

    //Send an IPC request to the server
    labstor::register_qp_request request(0);
    serversock_.SendMSG((void*)&request, sizeof(labstor::register_qp_request));

    //Receive SHMEM IPCs
    labstor::register_qp_reply reply;
    serversock_.RecvMSG((void*)&reply, sizeof(labstor::register_qp_reply));


}

void labstor::Client::IPCManager::CreateInternalQueues(int num_queues) {
}

void labstor::Client::IPCManager::PauseQueues() {
}

void labstor::Client::IPCManager::ResumeQueues() {
}