//
// Created by lukemartinlogan on 8/24/21.
//

//
// Created by lukemartinlogan on 8/20/21.
//

#include <cstdio>
#include <cstring>

#include <mpi.h>
#include <labstor/util/timer.h>
#include <labstor/shared_memory/boost.h>
#include <labstor/ipc/unordered_map.h>

int main(int argc, char **argv) {
    size_t id;
    int nprocs, rank;
    void *region;
    size_t num_buckets = 8;
    size_t region_size = labstor::ipc::unordered_map<size_t, size_t>::size(num_buckets);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    id = rank;

    if(rank == 0) {
        labstor::ipc::boost_shmem shmem;
        shmem.init("hi", region_size);
        region = shmem.get_address();
        labstor::ipc::unordered_map<size_t, size_t> map;
        map.init(region, region_size);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    labstor::ipc::boost_shmem shmem;
    shmem.open_rw("hi");
    region = shmem.get_address();
    labstor::ipc::unordered_map<size_t, size_t> map;
    map.open(region);

    MPI_Barrier(MPI_COMM_WORLD);
    for(int i = 0; i < 4; ++i) {
        map.create(rank*4 + i, i);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    for(int i = 0; i < 4; ++i) {
        size_t *val = map.find(rank*4 + i);
        printf("id=%d, val=%lu\n", rank*4 + i, *val);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        shmem.remove();
    }

    MPI_Finalize();
}