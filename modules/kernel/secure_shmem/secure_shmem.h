//
// Created by lukemartinlogan on 9/16/21.
//

#ifndef LABSTOR_SECURE_SHMEM_H
#define LABSTOR_SECURE_SHMEM_H

#include <labstor/types/basics.h>

#include <linux/types.h>
#include <linux/list.h>

#define SHMEM_ID "SHMEM_ID"

enum {
    RESERVE_SHMEM,
    GRANT_PID_SHMEM,
    FREE_SHMEM
};

struct shmem_region_info {
    struct list_head node;
    int region_id;
    size_t  size;
    void *vmalloc_ptr;
    bool user_owned;
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

struct shmem_request_netlink {
    struct labstor_netlink_header header;
    struct shmem_request rq;
};

struct shmem_ops {
    void* (*reserve_shmem)(size_t size, bool user_owned, int *new_region_id);
    void (*grant_pid_shmem)(int region_id, int pid);
    void (*free_shmem_region_by_id)(int region_id);
};

struct shmem_region_info *find_shmem_region(int region_id);

#endif //LABSTOR_SECURE_SHMEM_H
