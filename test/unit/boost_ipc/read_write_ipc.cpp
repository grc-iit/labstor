//
// Created by lukemartinlogan on 8/20/21.
//

#include <cstdio>
#include <cstring>

#include <mpi.h>
#include <labstor/util/timer.h>
#include <labstor/shared_memory/boost.h>

void ipc_write(int rank, int iter) {
    labstor::ipc::boost_shmem shmem;
    shmem.open_rw("hi");
    char *buf = (char*)shmem.get_address();

    labstor::Timer t;
    for(int i = 0; i < iter; ++i) {
        t.Resume();
        strcpy(buf, "hello");
        t.Pause();
    }
    if(rank == 0 || rank == 1) {
        printf("THROUGHPUT (ipc_write): %f\n", iter/t.GetSec());
    }
}

void ipc_read(int rank, int iter) {
    labstor::ipc::boost_shmem shmem;
    shmem.open_ro("hi");
    char *buf1 = (char*)shmem.get_address();
    char *buf2 = (char*)malloc(256);

    labstor::Timer t;
    for(int i = 0; i < iter; ++i) {
        t.Resume();
        strcpy(buf2, buf1);
        t.Pause();
    }
    if(rank == 0 || rank == 1) {
        printf("THROUGHPUT (ipc_read): %f\n", iter/t.GetSec());
    }
}

void simple_write(int rank, int iter) {
    char *buf = (char*)malloc(256);
    labstor::Timer t;
    for(int i = 0; i < iter; ++i) {
        t.Resume();
        strcpy(buf, "hello");
        t.Pause();
    }
    if(rank == 0 || rank == 1) {
        printf("THROUGHPUT (simple_write): %f\n", iter/t.GetSec());
    }
}

void simple_read(int rank, int iter) {
    char *buf1 = (char*)malloc(256);
    char *buf2 = (char*)malloc(256);
    strcpy(buf1, "hello");
    labstor::Timer t;
    for(int i = 0; i < iter; ++i) {
        t.Resume();
        strcpy(buf2, buf1);
        t.Pause();
    }
    if(rank == 0 || rank == 1) {
        printf("THROUGHPUT (simple_read): %f\n", iter/t.GetSec());
    }
}

int main(int argc, char **argv) {
    int nprocs, rank;
    int iter = 10000000;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    labstor::ipc::boost_shmem shmem;
    if(rank == 0) {
        try {
            shmem.remove("hi");
        } catch(...) {}
        shmem.init("hi", 256);
        char *buf = (char*)shmem.get_address();
        strcpy(buf, "hello");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank%2) {
        ipc_write(rank, iter);
    } else {
        ipc_read(rank, iter);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank%2) {
        simple_write(rank, iter);
    } else {
        simple_read(rank, iter);
    }

    if(rank == 0) {
        try {
            shmem.remove("hi");
        } catch(...) {}
    }

    MPI_Finalize();
}