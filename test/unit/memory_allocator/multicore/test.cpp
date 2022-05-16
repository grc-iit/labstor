//
// Created by lukemartinlogan on 9/21/21.
//

#include <mpi.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/allocator/private_shmem_allocator.h>
#include <labmods/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

uint32_t ncores = 8;
uint32_t page_size = 128;
uint32_t num_pages = 64;
size_t region_size = ncores * page_size * num_pages;
void *region;

labstor::GenericAllocator* private_allocator_init(int rank) {
    labstor::ipc::private_shmem_allocator *allocator = new labstor::ipc::private_shmem_allocator();
    if(rank == 0) {
        allocator->Init(region, region, region_size, page_size);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank != 0) {
        allocator->Attach(region, region);
    }
    return allocator;
}

labstor::GenericAllocator* multicore_allocator_init(int rank) {
    labstor::ipc::shmem_allocator *allocator = new labstor::ipc::shmem_allocator();
    if(rank == 0) {
        allocator->Init(region, region, region_size, page_size, 4);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank != 0) {
        allocator->Attach(region, region);
    }
    return allocator;
}

std::vector<void*> create_requests(int rank, labstor::GenericAllocator *allocator, int max) {
    void *page;
    int *intpg;
    std::vector<void*> pages;
    for(int i = 0; i < max; ++i) {
        page = allocator->Alloc(page_size, rank % ncores);
        if(page == nullptr) { break; }
        intpg = (int*)page;
        *intpg = rank*10000 + i;
        pages.emplace_back(page);
    }
    return std::move(pages);
}

void view_request_values(int rank, int nprocs, std::vector<void*> &pages) {
    MPI_Barrier(MPI_COMM_WORLD);
    for(int p = 0; p < nprocs; ++p) {
        if(rank == p) {
            for (int i = 0; i < pages.size(); ++i) {
                int *page = (int *)pages[i];
                printf("PAGE[%d,%d]: %d\n", rank, i, *page);
            }
            printf("\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

void multicore_test(labstor::GenericAllocator *allocator) {
    void *page;
    int *intpg, i = 0;
    std::vector<void*> pages;
    int nprocs, rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    //Allocate requests
    pages = create_requests(rank, allocator, ncores*num_pages);

    //View request values
    view_request_values(rank, nprocs, pages);

    //Free requests
    for(uint32_t i = 0; i < pages.size(); ++i) {
        allocator->Free(pages[i]);
    }

    //Allocate requests
    pages = create_requests(rank, allocator, pages.size());

    //View request values
    view_request_values(rank, nprocs, pages);
}

int main(int argc, char **argv) {
    int rank, region_id;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    labstor::GenericAllocator *alloc;
    auto netlink_client_ = LABSTOR_KERNEL_CLIENT;
    labstor::kernel::netlink::ShmemClient shmem_netlink;
    std::string allocator_type;

    //Allocator
    if(argc != 2) {
        printf("USAGE: ./test [allocator_type]\n");
        exit(1);
    }
    allocator_type = argv[1];

    //Create SHMEM region
    netlink_client_->Connect();
    if(rank == 0) {
        region_id = shmem_netlink.CreateShmem(region_size, true);
        if(region_id < 0) {
            printf("Failed to allocate SHMEM!\n");
            exit(1);
        }
        printf("Sending ID: %d\n", region_id);
    }
    MPI_Bcast(&region_id, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("REGION ID: %d\n", region_id);

    //Grant MPI rank access to region
    shmem_netlink.GrantPidShmem(getpid(), region_id);
    region = shmem_netlink.MapShmem(region_id, region_size);
    if(region == NULL) {
        printf("Failed to map shmem\n");
        exit(1);
    }
    printf("Mapped Region ID (rank=%d): %d %p\n", rank, region_id, region);

    //Perform the tests
    if(allocator_type == "PRIVATE") {
        multicore_test(private_allocator_init(rank));
    }
    if(allocator_type == "MULTICORE") {
        multicore_test(multicore_allocator_init(rank));
    }
    MPI_Barrier(MPI_COMM_WORLD);

    //Delete SHMEM region
    shmem_netlink.FreeShmem(region_id);

    MPI_Finalize();
}