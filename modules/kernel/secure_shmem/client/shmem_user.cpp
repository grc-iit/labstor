//
// Created by lukemartinlogan on 9/1/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <labstor/types/module.h>

/*
 * Use Case 1: Map X bytes of space between kernel and userspace; free when module unplugged
 * Use Case 2: Map X bytes of space between kernel and userspace; free when munmap called or process dies
 */

struct shmem_ops {
    void (*map_memory)
};

class SHMEMClient {
public:
    static struct shmem_ops* get_ops() { return &ops_; }

private:
    static struct shmem_ops {

    } ops_;
};

struct labstor::module {
    .module_id = "SHMEM",
    .process_request_fn=nullptr,
    .get_ops=SHMEMClient::get_ops
} module_;