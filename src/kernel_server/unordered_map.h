//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_KERNEL_SERVER_UNORDERED_MAP
#define LABSTOR_KERNEL_SERVER_UNORDERED_MAP

struct unordered_map {
    int nbuckets_;
    void **buckets_;
};

#endif //LABSTOR_KERNEL_SERVER_UNORDERED_MAP
