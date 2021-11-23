//
// Created by lukemartinlogan on 9/21/21.
//

#include <labstor/types/data_structures/shmem_ring_buffer.h>
#include <labstor/util/singleton.h>
#include <secure_shmem/netlink_client/shmem_user_netlink.h>

int main(int argc, char **argv) {
    size_t region_size = 4096;
    labstor::shmem_ring_buffer q;
    void *region = malloc(2*region_size);
    int *req_region = (int*)((char*)region + region_size);

    printf("REQUEST QUEUE START!\n");
    q.Init(region, region_size, 10);
    for(int i = 0; i < 10; ++i) {
        req_region[i] = i;
        q.Enqueue(req_region + i);
        printf("ENQUEUED REQUEST: %d\n", i);
    }

    int i = 0;
    while(i < 10){
        int* rq = (int*)q.Dequeue();
        if(!rq) {
            continue;
        }
        printf("DEQUEUED REQUEST: %d\n", *rq);
        ++i;
    }
}