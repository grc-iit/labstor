//
// Created by lukemartinlogan on 9/16/21.
//

#ifndef LABSTOR_IPC_TEST_H
#define LABSTOR_IPC_TEST_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>

#define IPC_TEST_MODULE_ID "IPC_TEST"

struct labstor_ipc_test_setup {
    struct labstor_request header;
    int region_id;
};

#endif //LABSTOR_IPC_TEST_H
