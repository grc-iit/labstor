
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

#ifndef LABSTOR_SECURE_SHMEM_H
#define LABSTOR_SECURE_SHMEM_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>

#define SHMEM_ID "SHMEM_ID"

enum {
    RESERVE_SHMEM,
    GRANT_PID_SHMEM,
    FREE_SHMEM
};

struct shmem_reserve_request {
    size_t size;
    bool user_owned;
};

struct shmem_grant_pid_shmem_request {
    int pid;
    int region_id;
};

struct free_shmem_request {
    int region_id;
};

struct secure_shmem_request {
    struct labstor_request header;
    union {
        struct shmem_reserve_request reserve;
        struct shmem_grant_pid_shmem_request grant;
        struct shmem_grant_pid_shmem_request free;
    };
};

#endif //LABSTOR_SECURE_SHMEM_H