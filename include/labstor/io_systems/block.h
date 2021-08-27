//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_BLOCK_H
#define LABSTOR_BLOCK_H

struct block_request {
    int op;
    int dev_id;
    size_t off;
    size_t len;
    void *buf;
};

struct block_io_ops {
    void read(struct block_request *rq);
    void write(struct block_request *rq);
};

#endif //LABSTOR_BLOCK_H
