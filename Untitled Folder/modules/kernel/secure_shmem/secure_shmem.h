//
// Created by lukemartinlogan on 9/16/21.
//

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
