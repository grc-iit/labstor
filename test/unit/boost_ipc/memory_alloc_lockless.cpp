//
// Created by lukemartinlogan on 8/24/21.
//

//
// Created by lukemartinlogan on 8/20/21.
//

#include <cstdio>
#include <cstring>

#include <omp.h>
#include <labstor/util/timer.h>
#include <labstor/shared_memory/boost.h>
#include <labstor/ipc/obj_allocator_lockless.h>

void test() {
    size_t id;
    int nprocs, rank;
    void *region;
    size_t obj_size = 4*(1<<10);
    size_t num_objs = 10;
    size_t num_buckets = 2;
    size_t region_size = 1024 + num_buckets*num_objs*obj_size;

    rank = omp_get_thread_num();
    id = rank;

    if(rank == 0) {
        labstor::ipc::boost_shmem shmem;
        shmem.init("hi", region_size);
        region = shmem.get_address();
        labstor::ipc::alloc::hashed_obj_allocator_lockless<void*> alloc;
        alloc.init(id, region, region_size, num_buckets, obj_size);
    }

#pragma omp barrier
    labstor::ipc::boost_shmem shmem;
    shmem.open_rw("hi");
    region = shmem.get_address();
    printf("REGION: %p\n", region);
    labstor::ipc::alloc::hashed_obj_allocator_lockless<void*> alloc;
    alloc.open(id, region);

#pragma omp barrier
    for(int i = 0; i < num_objs; ++i) {
        void *ptr = alloc.alloc(obj_size);
        printf("PTR: %p\n", ptr);
    }

    /*void *ptr = alloc.alloc(obj_size);
    printf("PTR: %p\n", ptr);*/

#pragma omp barrier
    if(rank == 0) {
        shmem.remove();
    }
}

int main(int argc, char **argv) {
    int nprocs = 16;
    omp_set_dynamic(0);
    #pragma omp parallel num_threads(nprocs)
    {
        test();
    }
}