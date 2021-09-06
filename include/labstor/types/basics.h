//
// Created by lukemartinlogan on 8/27/21.
//

#ifndef LABSTOR_BASICS_H
#define LABSTOR_BASICS_H

struct credentials {
    int pid;
    int uid;
    int gid;
};

struct pkg {
    char pkg_id[256];
    virtual void process_request_fn(void *request, struct credentials *creds);
    virtual void* get_ops(void);
    int req_size;
};

struct ipc {

};

#endif //LABSTOR_BASICS_H
