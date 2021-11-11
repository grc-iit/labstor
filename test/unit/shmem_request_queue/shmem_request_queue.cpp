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
    int rank, region_id;
    MPI_Init(&argc, &argv);
    size_t region_size = 4096;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    labstor::ipc::request_queue q;
    auto netlink_client__ = scs::Singleton<labstor::Kernel::NetlinkClient>::GetInstance();
    ShmemNetlinkClient shmem_netlink;

    //Create SHMEM region
    netlink_client__->Connect();
    if(rank == 0) {
        region_id = shmem_netlink.CreateShmem(region_size, true);
        if(region_id < 0) {
            printf("Failed to allocate SHMEM!\n");
            exit(1);
        }
        printf("Sending ID: %d\n", region_id);
        MPI_Send(&region_id, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    //Acquire region ID from rank 0
    if(rank == 1) {
        MPI_Recv(&region_id, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Receiving Region ID: %d\n", region_id);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    //Grant MPI rank access to region
    shmem_netlink.GrantPidShmem(getpid(), region_id);
    void *region = shmem_netlink.MapShmem(region_id, region_size);
    if(region == NULL) {
        printf("Failed to map shmem\n");
        exit(1);
    }
    printf("Mapped Region ID (rank=%d): %d %p\n", rank, region_id, region);

    //Initialize request queue and place request in the queue
    if(rank == 0) {
        q.Init(region, region_size, sizeof(simple_request));
        for(int i = 0; i < 10; ++i) {
            struct simple_request* rq = (struct simple_request*)q.Allocate(sizeof(struct simple_request));
            rq->hi = 12341;
            printf("ENQUEUEING REQUEST: %d\n", rq->hi);
            q.Enqueue(&rq->header);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);


    //Dequeue request and print its value
    if(rank == 1) {
        q.Attach(region);
        int i = 0;
        while(i < 10) {
            struct simple_request *rq = (struct simple_request *) q.Dequeue();
            if(!rq) { continue; }
            printf("RECEIVED REQUEST: %d\n", rq->hi);
            ++i;
        }
    }

    //Delete SHMEM region
    shmem_netlink.FreeShmem(region_id);

    MPI_Finalize();
}