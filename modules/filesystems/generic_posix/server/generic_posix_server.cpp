//
// Created by lukemartinlogan on 8/20/21.
//

#include <cstring>
#include <generic_posix.h>
#include "generic_posix_server.h"

void labstor::GenericPosix::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch(static_cast<Ops>(request->GetOp())) {
        case Ops::kOpen: {
            Open(qp, reinterpret_cast<generic_posix_open_request*>(request), creds);
            return;
        }
        case Ops::kClose: {
            Close(qp, reinterpret_cast<generic_posix_close_request*>(request), creds);
            return;
        }
        case Ops::kWrite:
        case Ops::kRead: {
            IOStart(qp, reinterpret_cast<generic_posix_io_request*>(request), creds);
            return;
        }
        case Ops::kIOComplete: {
            IOComplete(qp, reinterpret_cast<generic_posix_poll_io_request*>(request), creds);
            return;
        }
    }
}
int labstor::GenericPosix::Server::PriorSlash(char *path, int len) {
    int i = 0;
    for(i = len - 1; i >= 0; --i) {
       if(path[i] == '/') {
           return i+1;
       }
    }
    return 0;
}
void labstor::GenericPosix::Server::Open(labstor::ipc::queue_pair *qp, generic_posix_open_request *client_rq, labstor::credentials *creds) {
    char *path = client_rq->path_;
    int len = strlen(path);
    int fd = client_rq->fd_;
    uint32_t ns_id;
    uint64_t pid_fd = creds->pid << 32 + fd;
    labstor::Module *module;
    while(len > 0) {
        if(namespace_->GetIfExists(labstor::ipc::string(client_rq->path_, len), ns_id)) {
            module = namespace_->Get(ns_id);
            module->ProcessRequest(qp, reinterpret_cast<labstor::ipc::request*>(client_rq), creds);
            //[pid_fd] -> ns_id
            fd_table_->Set(pid_fd, ns_id);
            client_rq->Complete(LABSTOR_GENERIC_FS_SUCCESS);
            qp->Complete(client_rq);
            return;
        }
        len = PriorSlash(path, len);
    }
    client_rq->Complete(LABSTOR_GENERIC_FS_PATH_NOT_FOUND);
    qp->Complete(client_rq);
}
void labstor::GenericPosix::Server::Close(labstor::ipc::queue_pair *qp, generic_posix_close_request *client_rq, labstor::credentials *creds) {
    client_rq->SetCode(LABSTOR_GENERIC_FS_PATH_NOT_FOUND);
    qp->Complete(client_rq);
}
void labstor::GenericPosix::Server::IOStart(labstor::ipc::queue_pair *qp, generic_posix_io_request *client_rq, labstor::credentials *creds) {
}
void labstor::GenericPosix::Server::IOComplete(labstor::ipc::queue_pair *qp, generic_posix_poll_io_request *client_rq, labstor::credentials *creds) {
}

LABSTOR_MODULE_CONSTRUCT(labstor::GenericPosix::Server)