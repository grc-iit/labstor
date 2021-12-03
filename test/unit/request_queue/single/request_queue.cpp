//
// Created by lukemartinlogan on 9/21/21.
//

#include <labstor/types/data_structures/shmem_request_queue.h>
#include <modules/kernel/secure_shmem/netlink_client/shmem_user_netlink.h>

int main(int argc, char **argv) {
    size_t region_size = 4096;
    labstor::ipc::request_queue q;
    labstor::ipc::request *rq;
    void *region = malloc(2*region_size);
    labstor::ipc::request *req_region = (labstor::ipc::request*)((char*)region + region_size);

    printf("REQUEST QUEUE START!\n");
    q.Init(region, region_size, 10);
    for(int i = 0; i < 10; ++i) {
        req_region[i].ns_id_ = i;
        q.Enqueue(req_region + i);
        printf("ENQUEUED REQUEST[%lu]: %d\n", req_region + i, i);
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
        printf("DEQUEUED REQUEST[%lu]: %d\n", rq, rq->ns_id_);
        ++i;
    }
}