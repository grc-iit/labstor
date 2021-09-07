//
// Created by lukemartinlogan on 8/27/21.
//

#ifndef LABSTOR_BASICS_H
#define LABSTOR_BASICS_H

namespace labstor {

struct labstor_id {
    char id[256];
};

struct credentials {
    int pid;
    int uid;
    int gid;
};

struct labstor_pkg {
    struct labstor_id pkg_id;
    virtual void process_request_fn(struct queue_pair *qp, void *request, struct credentials *creds);
    virtual void *get_ops(void);
    int req_size;
};

struct ipc {

};

}

#endif //LABSTOR_BASICS_H
