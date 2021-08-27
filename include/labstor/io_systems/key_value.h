//
// Created by lukemartinlogan on 8/5/21.
//

#ifndef LABSTOR_KEY_VALUE_H
#define LABSTOR_KEY_VALUE_H

struct key_value_request {
    int op;
    union {
        struct key_value_load_request ;
    } rq;
};

struct key_value_io_ops {
    void put(struct block_request *rq);
    void get(struct block_request *rq);
};

#endif //LABSTOR_KEY_VALUE_H
