//
// Created by lukemartinlogan on 9/16/21.
//

#ifndef LABSTOR_SECURE_SHMEM_H
#define LABSTOR_SECURE_SHMEM_H

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

struct shmem_request {
    int op;
    union {
        struct shmem_reserve_request reserve;
        struct shmem_grant_pid_shmem_request grant;
        struct shmem_grant_pid_shmem_request free;
    };
};

struct shmem_ops {
    void* (*reserve_shmem)(size_t size, bool user_owned, int *new_region_id);
    void (*grant_pid_shmem)(int region_id, int pid);
    void (*free_shmem_region_by_id)(int region_id);
};

#endif //LABSTOR_SECURE_SHMEM_H
