
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <labstor/userspace/server/macros.h>
#include <labstor/kernel/client/kernel_client.h>
#include <secure_shmem/secure_shmem.h>
#include <labmods/secure_shmem/netlink_client/secure_shmem_client_netlink.h>

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