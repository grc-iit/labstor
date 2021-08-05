//
// Created by lukemartinlogan on 8/5/21.
//

#ifndef LABSTOR_KEY_VALUE_H
#define LABSTOR_KEY_VALUE_H

struct key_value_request {
    int op;
    union {
        struct key_value_load_request ;
    };
};

struct key_value_io_ops {
    void read(struct block_request *rq);
    void write(struct block_request *rq);
};

#endif //LABSTOR_KEY_VALUE_H
