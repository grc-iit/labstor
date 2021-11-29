//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_ARRAY_H
#define LABSTOR_ARRAY_H

#include <linux/types.h>
#include <labstor/types/basics.h>

struct labstor_array_header* {
    uint32_t length_;
};

struct labstor_array {
    labstor_array_header *header_;
    void *arr_;
};

#endif //LABSTOR_ARRAY_H
