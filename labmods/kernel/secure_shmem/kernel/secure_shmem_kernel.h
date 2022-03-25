//
// Created by lukemartinlogan on 12/4/21.
//

#ifndef LABSTOR_SECURE_SHMEM_KERNEL_H
#define LABSTOR_SECURE_SHMEM_KERNEL_H

#include <secure_shmem/secure_shmem.h>
#include <linux/list.h>
#include <linux/types.h>

struct shmem_region_info {
    struct list_head node;
    int region_id;
    size_t size;
    void *vmalloc_ptr;
    bool user_owned;
};

struct shmem_region_info *labstor_find_shmem_region_info(int region_id);
void *labstor_find_shmem_region(int region_id);

#endif //LABSTOR_SECURE_SHMEM_KERNEL_H
