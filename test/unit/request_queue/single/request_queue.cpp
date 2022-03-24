//
// Created by lukemartinlogan on 9/21/21.
//

#include "labstor/types/data_structures/c/shmem_request_queue.h"
#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

int main(int argc, char **argv) {
    uint32_t num_requests = 171;
    uint32_t queue_size = labstor::ipc::request_queue::GetSize(num_requests);
    uint32_t req_buf_size = num_requests * sizeof(labstor::ipc::request);
    labstor::ipc::request_queue q;
    labstor::ipc::request *rq;
    labstor::ipc::qtok_t qtok;
    void *region = malloc(queue_size + req_buf_size);
    labstor::ipc::request *req_region = (labstor::ipc::request*)((char*)region + queue_size);

    printf("REQUEST QUEUE START!\n");
    q.Init(region, region, queue_size, 10);
    printf("Max Depth: %d vs %d\n", q.GetMaxDepth(), num_requests);
    if(q.GetMaxDepth() != num_requests) {
        printf("Max depth calculation is off\n");
        exit(1);
    }
    for(int i = 0; i < 10; ++i) {
        req_region[i].ns_id_ = i+1;
        q.Enqueue(req_region + i, qtok);
        printf("ENQUEUED REQUEST[%lu]: %d\n", (size_t)(req_region + i), i);
    }
    printf("\n");

    int i = 0;
    while(i < 10){
        if(!q.Dequeue(rq)) {
            continue;
        }
        if(!rq) {
            continue;
        }
        printf("DEQUEUED REQUEST[%lu]: %d\n", (size_t)(rq), rq->ns_id_);
        ++i;
    }
}