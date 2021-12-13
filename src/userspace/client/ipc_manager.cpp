//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/userspace/client/client.h>
#include <labstor/userspace/types/messages.h>
#include <labstor/constants/debug.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/socket.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

void labstor::Client::IPCManager::Connect() {
    AUTO_TRACE("labstor::Client::IPCManager::Connect")
    ssize_t ret;
    int serverfd;
    struct sockaddr_un client_addr;
    struct sockaddr_un server_addr;
    void *region;

    //Get our pid
    pid_ = getpid();

    //Create UDP socket
    TRACEPOINT("CREATE UDP SOCKET")
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
    TRACEPOINT("Set server address and connect")
    memset(&server_addr, 0x0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, TRUSTED_SERVER_PATH, sizeof(server_addr.sun_path)-1);
    if (connect(serverfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un)) == -1) {
        close(serverfd);
        throw UNIX_CONNECT_FAILED.format(strerror(errno));
    }
    serversock_.SetFd(serverfd);

    //Receive SHMEM region
    TRACEPOINT("Get SHMEM region")
    labstor::ipc::setup_reply reply;
    serversock_.RecvMSG(&reply, sizeof(reply));

    //Attach SHMEM allocator
    TRACEPOINT("Attach SHMEM allocator")
    labstor::ipc::shmem_allocator *shmem_alloc;
    region = labstor::kernel::netlink::ShmemClient::MapShmem(reply.region_id, reply.region_size);
    shmem_alloc = new labstor::ipc::shmem_allocator();
    shmem_alloc->Attach(region, region);
    shmem_alloc_ = shmem_alloc;
    TRACEPOINT("SHMEM allocator", (size_t)shmem_alloc->GetRegion())

    //Initialize internal allocator
    TRACEPOINT("Initialize internal allocator")
    labstor::ipc::shmem_allocator *internal_alloc;
    internal_alloc = new labstor::ipc::shmem_allocator();
    internal_alloc->Init(region=malloc(reply.region_size), region, reply.region_size, reply.request_unit);
    private_alloc_ = internal_alloc;
    TRACEPOINT("Internal allocator", (size_t)internal_alloc->GetRegion())

    //Create the SHMEM queues
    TRACEPOINT("Create SHMEM queues")
    CreateQueuesSHMEM(reply.concurrency, reply.queue_size);
    CreatePrivateQueues(reply.concurrency, reply.queue_size);
}

void labstor::Client::IPCManager::CreateQueuesSHMEM(int num_queues, int queue_size) {
    AUTO_TRACE("labstor::Client::IPCManager::CreateQueuesSHMEM")
    labstor::ipc::register_qp_request request(num_queues);
    labstor::ipc::register_qp_reply reply;
    labstor::ipc::queue_pair_ptr *qps = (labstor::ipc::queue_pair_ptr *)malloc(request.GetQueueArrayLength());

    //Allocate SHMEM queues for the client
    for(int i = 0; i < num_queues; ++i) {
        labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                pid_);
        void *sq_region = shmem_alloc_->Alloc(queue_size);
        void *cq_region = shmem_alloc_->Alloc(queue_size);
        shmem_qps_.emplace_back(new labstor::ipc::queue_pair(qid, shmem_alloc_->GetRegion(), sq_region, queue_size, cq_region, queue_size));
        qps[i].Init(qid, sq_region, cq_region, shmem_alloc_->GetRegion());
    }

    //Send an IPC request to the server
    serversock_.SendMSG((void*)&request, sizeof(labstor::ipc::register_qp_request));
    serversock_.SendMSG((void*)qps, request.GetQueueArrayLength());

    //Receive SHMEM IPCs
    serversock_.RecvMSG((void*)&reply, sizeof(labstor::ipc::register_qp_reply));
    free(qps);
}

void labstor::Client::IPCManager::CreatePrivateQueues(int num_queues, int queue_size) {
    AUTO_TRACE("labstor::Client::IPCManager::CreatePrivateQueues")
    for(int i = 0; i < num_queues; ++i) {
        labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_PRIVATE | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                pid_);
        void *sq_region = private_alloc_->Alloc(queue_size);
        void *cq_region = private_alloc_->Alloc(queue_size);
        private_qps_.emplace_back(new labstor::ipc::queue_pair(qid,private_alloc_->GetRegion(), sq_region, queue_size, cq_region, queue_size));
    }
}

void labstor::Client::IPCManager::PauseQueues() {
}

void labstor::Client::IPCManager::ResumeQueues() {
}