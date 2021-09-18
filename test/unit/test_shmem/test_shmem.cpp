//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/util/singleton.h>
#include <labstor/kernel_client/kernel_client.h>
#include <secure_shmem/secure_shmem.h>
#include <secure_shmem/netlink_client/shmem_user_netlink.h>

int main() {
    int region_id;
    size_t region_size = 128;
    char *region;
    auto labstor_kernel_context = scs::Singleton<labstor::LabStorKernelClientContext>::GetInstance();
    labstor_kernel_context->Connect();

    ShmemNetlinkClient shmem;

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