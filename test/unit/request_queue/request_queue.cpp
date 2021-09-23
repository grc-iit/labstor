//
// Created by lukemartinlogan on 9/21/21.
//

#include <mpi.h>
#include <labstor/ipc/simple_allocator.h>
#include <labstor/ipc/request_queue.h>
#include <labstor/util/singleton.h>
#include <secure_shmem/netlink_client/shmem_user_netlink.h>

struct simple_request {
    struct labstor::ipc::request header;
    int hi;
};

int main(int argc, char **argv) {
    int rank;
    size_t region_size = 4096;
    labstor::ipc::request_queue q;
    void *region = malloc(region_size);

    q.Init(region, region_size, sizeof(simple_request));
    for(int i = 0; i < 10; ++i) {
        struct simple_request* rq = (struct simple_request*)q.Allocate(sizeof(struct simple_request));
        rq->hi = 12341 + i;
        q.Enqueue(&rq->header);
        printf("ENQUEUED REQUEST: %d\n", rq->hi);
    }

    int i = 0;
    while(i < 10){
        struct simple_request* rq = (struct simple_request*)q.Dequeue();
        if(!rq) {
            continue;
        }
        printf("DEQUEUED REQUEST: %d\n", rq->hi);
        ++i;
    }
}