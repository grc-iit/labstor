//
// Created by lukemartinlogan on 11/18/21.
//

#ifndef LABSTOR_MACROS_H
#define LABSTOR_MACROS_H

#define KERNEL_PID 0
#define LABSTOR_REGION_SUB_POS(ptr, region) (labstor::off_t)((size_t)ptr - (size_t)region)
#define LABSTOR_REGION_SUB_NEG(ptr, region) -(labstor::off_t)((size_t)region - (size_t)ptr)
#define LABSTOR_REGION_SUB(ptr, region) ((size_t)ptr >= (size_t)region ? LABSTOR_REGION_SUB_POS(ptr,region) : LABSTOR_REGION_SUB_NEG(ptr,region))
#define LABSTOR_REGION_ADD(off, region) (void*)((char*)region + off)

#endif //LABSTOR_MACROS_H
