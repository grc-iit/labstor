//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/util/singleton.h>
#include <labstor/kernel_client/kernel_client.h>
#include <secure_shmem/secure_shmem.h>
#include <secure_shmem/client/shmem_user_netlink.h>

int main() {
    int region_id;
    char *region;
    auto labstor_kernel_context = scs::Singleton<labstor::LabStorKernelClientContext>::GetInstance();
    labstor_kernel_context->Connect();

    ShmemNetlinkClient shmem;
    region_id = shmem.CreateShmem(128, true);
    printf("REGION ID: %d\n", region_id);
    if(region_id >= 0) {
        shmem.FreeShmem(region_id);
    }
}