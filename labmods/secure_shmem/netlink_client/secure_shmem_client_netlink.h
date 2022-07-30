
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

#ifndef LABSTOR_SECURE_SHMEM_NETLINK_H
#define LABSTOR_SECURE_SHMEM_NETLINK_H

#include <labstor/kernel/client/macros.h>
#include <labstor/kernel/client/kernel_client.h>

#define SHMEM_CHRDEV "/dev/labstor_shared_shmem0"

namespace labstor::kernel::netlink {

class ShmemClient {
private:
    LABSTOR_KERNEL_CLIENT_T kernel_client_;
    int page_size_;
public:
    ShmemClient() {
        kernel_client_ = LABSTOR_KERNEL_CLIENT;
        page_size_ = getpagesize();
    }
    int CreateShmem(size_t region_size, bool user_owned);
    int GrantPidShmem(int pid, int region_id);
    int FreeShmem(int region_id);
    static void *MapShmem(int region_id, size_t region_size);
    static void UnmapShmem(void *region, size_t region_size);
};

}

#endif //LABSTOR_SECURE_SHMEM_NETLINK_H