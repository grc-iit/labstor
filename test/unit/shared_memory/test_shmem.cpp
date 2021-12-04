//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/userspace/server/macros.h>
#include <labstor/kernel/client/kernel_client.h>
#include <secure_shmem/secure_shmem.h>
#include <modules/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

int main() {
    int region_id;
    size_t region_size = 128;
    char *region;
    auto netlink_client_ = LABSTOR_KERNEL_CLIENT;
    netlink_client_->Connect();

    labstor::kernel::netlink::ShmemClient shmem;

    //Create a shared memory region
    region_id = shmem.CreateShmem(region_size, true);
    printf("REGION ID: %d\n", region_id);
    if(region_id < 0) {
        printf("Failed to allocate region");
        return -1;
    }

    //Grant access to the shared memory region
    printf("PID: %d\n", getpid());
    shmem.GrantPidShmem(getpid(), region_id);

    //Map the shared memory region
    region = (char*)shmem.MapShmem(region_id, region_size);
    printf("REGION: %p\n", region);
    if(!region) {
        perror("Can't open shmem");
        return -1;
    }
    region[0] = 'h';

    //Free the shared memory region
    shmem.FreeShmem(region_id);
}