//
// Created by lukemartinlogan on 11/25/21.
//

#include <mpi.h>
#include <labstor/kernel/client/macros.h>
#include <labstor/kernel/client/kernel_client.h>
#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>
#include <labstor/types/data_structures/shmem_unordered_map_uint32_t_uint32_t.h>
#include <cstdio>

int main(int argc, char **argv) {
    int rank, region_id;
    MPI_Init(&argc, &argv);
    uint32_t region_size = (1<<20);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    labstor::ipc::int_map_uint32_t_uint32_t map;

    auto netlink_client_ = LABSTOR_KERNEL_CLIENT;
    labstor::kernel::netlink::ShmemClient shmem_netlink;
    printf("Starting test\n");

    //Create SHMEM region
    netlink_client_->Connect();
    printf("Netlink client connected\n");
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
        map.Init(region, region_size, 16);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank != 0) {
        map.Attach(region);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    //Place requests in the queue
    if(rank == 0) {
        for(int i = 0; i < 10000; ++i) {
            if(map.Set(i, i)) {
                printf("ENQUEUEING REQUEST: %d\n", i);
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);


    //Dequeue request and print its value
    if(rank == 1) {
        uint32_t i = 0, v;
        while(i < 10000) {
            if(!map.Find(i, v)) { continue; }
            map.Remove(i);
            printf("RECEIVED REQUEST[%d]: %d\n", i, v);
            ++i;
        }
    }

    //Delete SHMEM region
    shmem_netlink.FreeShmem(region_id);

    MPI_Finalize();
}
