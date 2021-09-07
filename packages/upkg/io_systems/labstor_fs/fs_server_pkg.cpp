//
// Created by lukemartinlogan on 8/20/21.
//

struct labstor_fs_server_pkg {
    struct labstor_id pkg_id;
    void process_request_fn(struct posix_request *request, struct credentials *creds);
    void *get_ops(void) {}
    int req_size;
};