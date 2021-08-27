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
#include <labstor/ipc/obj_allocator_lock.h>

int main(int argc, char **argv) {
    size_t id;
    int nprocs, rank;
    void *region;
    size_t obj_size = 4*(1<<10);
    size_t num_objs = 10;
    size_t niter = num_objs*2;
    size_t num_buckets = 2;
    size_t region_size = 1024 + num_buckets*num_objs*obj_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    id = rank;

    if(rank == 0) {
        labstor::ipc::boost_shmem shmem;
        shmem.init("hi", region_size);
        region = shmem.get_address();
        labstor::ipc::alloc::hashed_obj_allocator_lock<void*> alloc;
        alloc.init(id, region, region_size, num_buckets, obj_size);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    labstor::ipc::boost_shmem shmem;
    shmem.open_rw("hi");
    region = shmem.get_address();
    labstor::ipc::alloc::hashed_obj_allocator_lock<void*> alloc;
    alloc.open(id, region);

    MPI_Barrier(MPI_COMM_WORLD);
    for(int i = 0; i < num_objs; ++i) {
        void *ptr = alloc.alloc(obj_size);
        printf("PTR: %p\n", ptr);
    }

    void *ptr = alloc.alloc(obj_size);
    printf("PTR: %p\n", ptr);

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        shmem.remove();
    }

    MPI_Finalize();
}