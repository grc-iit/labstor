//
// Created by lukemartinlogan on 9/16/21.
//

#ifndef LABSTOR_SECURE_SHMEM_H
#define LABSTOR_SECURE_SHMEM_H

enum {
    SHMEM_RESERVE,
    GRANT_PID_SHMEM
};

struct shmem_request {
    int op;
    int pid;
    size_t size;
};

#endif //LABSTOR_SECURE_SHMEM_H
