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
#include <labstor/types/allocator/segment_allocator.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>
#include <sys/sysinfo.h>

void labstor::Client::IPCManager::Connect() {
    AUTO_TRACE("")
    ssize_t ret;
    int serverfd;
    struct sockaddr_un client_addr;
    struct sockaddr_un server_addr;
    void *region;

    if(IsConnected()) {
        return;
    }

    //Get our pid
    pid_ = getpid();
    n_cpu_ = get_nprocs_conf();

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
    TRACEPOINT("Receive reply", "region_id", reply.region_id, "region_size", reply.region_size, "queue_size", reply.queue_region_size, "queue_depth", reply.queue_depth)
    region = labstor::kernel::netlink::ShmemClient::MapShmem(reply.region_id, reply.region_size);

    //Initialize SHMEM request allocator
    TRACEPOINT("Attach SHMEM allocator")
    labstor::ipc::shmem_allocator *shmem_alloc;
    shmem_alloc = new labstor::ipc::shmem_allocator();
    shmem_alloc->Init(region, region, reply.request_region_size, reply.request_unit, n_cpu_);
    shmem_alloc_ = shmem_alloc;
    TRACEPOINT("SHMEM allocator", (size_t)shmem_alloc->GetRegion())

    //Initialize SHMEM queue allocator
    qp_alloc_ = new labstor::segment_allocator();
    qp_alloc_->Attach(
            LABSTOR_REGION_ADD(reply.request_region_size, region), reply.queue_region_size);

    //Initialize internal allocator
    TRACEPOINT("Initialize internal allocator")
    labstor::ipc::shmem_allocator *private_alloc;
    private_alloc = new labstor::ipc::shmem_allocator();
    private_alloc->Init(region=malloc(reply.region_size), region, reply.region_size, reply.request_unit, n_cpu_);
    private_alloc_ = private_alloc;
    TRACEPOINT("Internal allocator", (size_t)private_alloc->GetRegion())

    //Create the SHMEM queues
    TRACEPOINT("Create SHMEM queues")
    CreateQueuesSHMEM(reply.num_queues, reply.queue_depth);
    CreatePrivateQueues(n_cpu_, reply.queue_depth);

    //Mark as connected
    is_connected_ = true;
}

void labstor::Client::IPCManager::CreateQueuesSHMEM(int num_queues, int depth) {
    AUTO_TRACE("")
    labstor::ipc::register_qp_request request(num_queues);
    labstor::ipc::register_qp_reply reply;
    labstor::ipc::queue_pair_ptr *qps = (labstor::ipc::queue_pair_ptr *)malloc(request.GetQueueArrayLength());
    uint32_t request_queue_size = labstor::ipc::request_queue::GetSize(depth);
    uint32_t request_map_size = labstor::ipc::request_map::GetSize(depth);

    //Allocate SHMEM queues for the client
    for(int i = 0; i < num_queues; ++i) {
        labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                pid_);
        void *sq_region = qp_alloc_->Alloc(request_queue_size);
        void *cq_region = qp_alloc_->Alloc(request_map_size);
        TRACEPOINT("Creating queue", i, qid);
        shmem_qps_.emplace_back(
                new labstor::ipc::queue_pair(qid, shmem_alloc_->GetRegion(), sq_region, request_queue_size, cq_region, request_map_size));
        TRACEPOINT("Created queue", i, qid);
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
    AUTO_TRACE("")
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