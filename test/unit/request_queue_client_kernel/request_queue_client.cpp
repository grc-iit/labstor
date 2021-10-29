//
// Created by lukemartinlogan on 9/21/21.
//

#include <labstor/ipc/simple_allocator.h>
#include <labstor/ipc/request_queue.h>
#include <labstor/util/singleton.h>
#include <secure_shmem/netlink_client/shmem_user_netlink.h>

struct simple_request {
    struct labstor::ipc::request header;
    int hi;
};

int main(int argc, char **argv) {
    int region_id;
    size_t region_size = 4096;
    labstor::ipc::request_queue q;
    auto labstor_kernel_context_ = scs::Singleton<labstor::LabStorKernelClientContext>::GetInstance();
    ShmemNetlinkClient shmem_netlink;
    struct simple_request* rq;

    //Create SHMEM region
    labstor_kernel_context_->Connect();
    region_id = shmem_netlink.CreateShmem(region_size, true);
    if(region_id < 0) {
        printf("Failed to allocate SHMEM!\n");
        exit(1);
    }
    printf("Sending ID: %d\n", region_id);

    //Grant MPI rank access to region
    shmem_netlink.GrantPidShmem(getpid(), region_id);
    void *region = shmem_netlink.MapShmem(region_id, region_size);
    if(region == NULL) {
        printf("Failed to map shmem\n");
        exit(1);
    }
    printf("Mapped Region ID (rank=%d): %d %p\n", rank, region_id, region);

    //Initialize request queue and place request in the queue
    q.Init(region, region_size, sizeof(simple_request));
    for(int i = 0; i < 10; ++i) {
        rq = (struct simple_request*)q.Allocate(sizeof(struct simple_request));
        rq->data = 12341;
        printf("ENQUEUE IN USER: %d\n", rq->data);
        q.Enqueue(&rq->header);
    }

    //Send dequeue message to kernel module
    int code;
    struct request_queue_test_request_netlink dqmsg = {
            .header.module_id = REQUEST_QUEUE_TEST_ID,
            .rq.op = REQUEST_QUEUE_TEST_PKG_DEQUEUE,
            .rq.region_id = region_id
    };
    labstor_kernel_context_->SendMSG(&dqmsg, sizeof(dqmsg));
    labstor_kernel_context_->RecvMSG(&code, sizeof(code));
    printf("Received the following code: %d\n", code);

    //Dequeue messages from kernel module
    while(rq = (struct simple_request*)q.Dequeue()) {
        printf("DEQUEUE IN USER: %d\n", rq->data);
    }

    //Delete SHMEM region
    //shmem_netlink.FreeShmem(region_id);
}