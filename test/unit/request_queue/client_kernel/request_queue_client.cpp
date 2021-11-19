//
// Created by lukemartinlogan on 9/21/21.
//

#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/shmem_request_queue.h>
#include <labstor/util/singleton.h>
#include <secure_shmem/netlink_client/shmem_user_netlink.h>
#include "request_queue_test.h"

int main(int argc, char **argv) {
    /*int region_id;
    size_t region_size = 4096;
    labstor::request_queue q;
    auto netlink_client__ = scs::Singleton<labstor::Kernel::NetlinkClient>::GetInstance();
    ShmemNetlinkClient shmem_netlink;
    struct simple_request* rq;

    //Create SHMEM region
    netlink_client__->Connect();
    region_id = shmem_netlink.CreateShmem(region_size, true);
    if(region_id < 0) {
        printf("Failed to allocate SHMEM!\n");
        exit(1);
    }
    printf("Sending ID: %d\n", region_id);

    //Grant process access to region
    shmem_netlink.GrantPidShmem(getpid(), region_id);
    void *region = shmem_netlink.MapShmem(region_id, region_size);
    if(region == NULL) {
        printf("Failed to map shmem\n");
        exit(1);
    }
    printf("Mapped Region ID: %d %p\n", region_id, region);

    //Initialize request queue and place request in the queue
    q.Init(region, region_size, sizeof(simple_request));
    for(int i = 0; i < 10; ++i) {
        rq = (struct simple_request*)q.Allocate(sizeof(struct simple_request));
        rq->data = 12341 + i;
        printf("ENQUEUE IN USER: %d\n", rq->data);
        q.Enqueue(&rq->header);
    }

    //Send dequeue message to kernel module
    int code;
    struct request_queue_test_request_netlink dqmsg;
    strcpy(dqmsg.header.module_id.key, REQUEST_QUEUE_TEST_ID);
    dqmsg.rq.op = REQUEST_QUEUE_TEST_PKG_DEQUEUE;
    dqmsg.rq.region_id = region_id;
    netlink_client__->SendMSG(&dqmsg, sizeof(dqmsg));
    netlink_client__->RecvMSG(&code, sizeof(code));
    printf("Received the following code: %d\n", code);

    //Dequeue messages from kernel module
    while(rq = (struct simple_request*)q.Dequeue()) {
        printf("DEQUEUE IN USER: %d\n", rq->data);
    }

    //Delete SHMEM region
    //shmem_netlink.FreeShmem(region_id);*/
}