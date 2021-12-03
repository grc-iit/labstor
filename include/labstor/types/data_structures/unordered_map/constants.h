//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_UNORDERED_MAP_CONSTANTS_H
#define LABSTOR_UNORDERED_MAP_CONSTANTS_H

#include <labstor/types/basics.h>

static const labstor_off_t null1_mark = 1 << 31;
static const labstor_off_t null1_null = ~((uint32_t)0) ^ null1_mark;

static const labstor_off_t null0_mark = 1 << (sizeof(labstor_off_t)*8 - 1);
static const labstor_off_t null0_null = 0;


#endif //LABSTOR_UNORDERED_MAP_CONSTANTS_H
