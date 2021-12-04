//
// Created by lukemartinlogan on 9/21/21.
//

#include <mpi.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/shmem_request_queue.h>
#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

struct simple_request : public labstor::ipc::request {
    int hi;
};

int main(int argc, char **argv) {
    int rank, region_id;
    MPI_Init(&argc, &argv);
    uint32_t region_size = (1<<20);
    uint32_t alloc_region_size = (1<<19);
    uint32_t queue_region_size = (1<<19);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    labstor::ipc::request_queue q;
    labstor::ipc::shmem_allocator alloc;
    auto netlink_client_ = LABSTOR_KERNEL_CLIENT;
    ShmemNetlinkClient shmem_netlink;

    //Create SHMEM region
    netlink_client_->Connect();
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

    //Initialize memory allocator and queue
    if(rank == 0) {
        alloc.Init(region, alloc_region_size, 256);
        region = alloc.GetNextSection();
        q.Init(region, queue_region_size, 1);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank != 0) {
        alloc.Attach(region);
        region = alloc.GetNextSection();
        q.Attach(region);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    //Place requests in the queue
    if(rank == 0) {
        for(int i = 0; i < 10; ++i) {
            simple_request* rq = (simple_request*)alloc.Alloc(sizeof(simple_request), 0);
            rq->hi = 12341 + i;
            printf("ENQUEUEING REQUEST: %d\n", rq->hi);
            q.Enqueue(rq);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);


    //Dequeue request and print its value
    if(rank != 0) {
        int i = 0;
        while(i < 10) {
            labstor::ipc::request *rq_uncast;
            simple_request *rq;
            if(!q.Dequeue(rq_uncast)) { continue; }
            rq = (simple_request*)rq_uncast;
            printf("RECEIVED REQUEST: %d\n", rq->hi);
            ++i;
        }
    }

    //Delete SHMEM region
    shmem_netlink.FreeShmem(region_id);

    MPI_Finalize();
}