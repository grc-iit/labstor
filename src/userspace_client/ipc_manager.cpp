//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/types/messages.h>
#include <labstor/util/singleton.h>
#include <labstor/util/errors.h>
#include <labstor/types/basics.h>
#include <labstor/types/socket.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/userspace_client/client.h>
#include <labstor/userspace_client/ipc_manager.h>
#include <secure_shmem/netlink_client/shmem_user_netlink.h>

void labstor::Client::IPCManager::Connect() {
    ssize_t ret;
    int serverfd;
    struct sockaddr_un client_addr;
    struct sockaddr_un server_addr;

    //Get our pid
    pid_ = getpid();

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
    labstor::ipc::setup_reply reply;
    serversock_.RecvMSG(&reply, sizeof(reply));

    //Attach SHMEM allocator
    labstor::ipc::shmem_allocator *shmem_alloc;
    void *region = ShmemNetlinkClient::MapShmem(reply.region_id, reply.region_size);
    shmem_alloc = new labstor::ipc::shmem_allocator();
    shmem_alloc->Attach(region);
    shmem_alloc_ = shmem_alloc;

    //Initialize internal allocator
    labstor::ipc::shmem_allocator *internal_alloc;
    internal_alloc = new labstor::ipc::shmem_allocator();
    internal_alloc->Attach(malloc(reply.region_size));
    private_alloc_ = internal_alloc;

    //Create the SHMEM queues
    CreateQueuesSHMEM(reply.concurrency, reply.queue_size);
    CreatePrivateQueues(reply.concurrency, reply.queue_size);
}

void labstor::Client::IPCManager::CreateQueuesSHMEM(int num_queues, int queue_size) {
    uint32_t qp_size = sizeof(labstor::ipc::queue_pair)*num_queues;
    labstor::ipc::queue_pair_ptr *qps = (labstor::ipc::queue_pair_ptr *)malloc(qp_size);

    //Allocate SHMEM queues for the client
    for(int i = 0; i < num_queues; ++i) {
        uint32_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                pid_);
        void *sq_region = shmem_alloc_->Alloc(queue_size);
        void *cq_region = shmem_alloc_->Alloc(queue_size);
        shmem_qps_.emplace_back(qid, sq_region, queue_size, cq_region, queue_size);
        qps[i].Init(sq_region, cq_region, shmem_alloc_->GetRegion());
    }

    //Send an IPC request to the server
    labstor::ipc::register_qp_request request(0);
    serversock_.SendMSG((void*)&request, sizeof(labstor::ipc::register_qp_request));
    serversock_.SendMSG((void*)qps, qp_size);

    //Receive SHMEM IPCs
    labstor::ipc::register_qp_reply reply;
    serversock_.RecvMSG((void*)&reply, sizeof(labstor::ipc::register_qp_reply));
    free(qps);
}

void labstor::Client::IPCManager::CreatePrivateQueues(int num_queues, int queue_size) {
    for(int i = 0; i < num_queues; ++i) {
        uint32_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                pid_);
        void *sq_region = private_alloc_->Alloc(queue_size);
        void *cq_region = private_alloc_->Alloc(queue_size);
        private_qps_.emplace_back(qid, sq_region, queue_size, cq_region, queue_size);
    }
}

void labstor::Client::IPCManager::PauseQueues() {
}

void labstor::Client::IPCManager::ResumeQueues() {
}