//
// Created by lukemartinlogan on 8/20/21.
//

#include "general_fs.h"

void generic_posix_process_request(struct posix_request *rq) {
    switch(rq->op) {
        case POSIX_OPEN_RQ:
            generic_posix_fs_open(rq->open.path, rq->open.flags, rq->open.mode);
            break;
        case POSIX_CLOSE_RQ:
            break;
        case POSIX_READ_RQ:
            break;
        case POSIX_WRITE_RQ:
            break;
    }
}

int generic_posix_fs_open(const char *pathname, int flags, mode_t mode) {
    if(!namespace_) {
        namespace_ = labstor::ipc::unordered_map<struct labstor_name>
        //Load filesystem lib from DLL
    }
    int pathlen = sizeof(pathname);
    for(int i = pathlen - 1; i >= 0; --i) {
    }
    //Get the metadata object for device
    //Call specific open() implementation
}
