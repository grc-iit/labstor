//
// Created by lukemartinlogan on 11/24/21.
//

#ifndef LABSTOR_SHMEM_UNORDERED_MAP_H
#define LABSTOR_SHMEM_UNORDERED_MAP_H

#include "labstor_array.h"

struct labstor_unordered_map {
    struct labstor_array buckets_;
    struct labstor_array overflow_;
};

#endif //LABSTOR_SHMEM_UNORDERED_MAP_H
