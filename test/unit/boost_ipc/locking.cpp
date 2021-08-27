//
// Created by lukemartinlogan on 8/20/21.
//

#include <cstdio>
#include <cstring>

#include <mpi.h>
#include <labstor/util/timer.h>
#include <labstor/shared_memory/shared_memory.h>
#include <labstor/ipc/obj_allocator_lock.h>

int main(int argc, char **argv) {
    int nprocs, rank;
    int iter = 10000000;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);



    MPI_Finalize();
}