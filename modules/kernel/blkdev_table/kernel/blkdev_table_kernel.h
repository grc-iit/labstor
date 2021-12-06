//
// Created by lukemartinlogan on 9/14/21.
//

#ifndef LABSTOR_BLKDEV_REGISTER_H
#define LABSTOR_BLKDEV_REGISTER_H

#include <blkdev_table/blkdev_table.h>

extern struct kmem_cache *page_cache;
struct block_device* labstor_get_bdev(int bdev_id);

#endif //LABSTOR_BLKDEV_REGISTER_H
