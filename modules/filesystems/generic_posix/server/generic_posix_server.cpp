//
// Created by lukemartinlogan on 8/20/21.
//

#include <cstring>
#include <generic_posix.h>
#include "generic_posix_server.h"

#define PID_FD(pid, fd) (((uint64_t)pid<<32) + fd)

void labstor::GenericPosix::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("")
    switch(static_cast<Ops>(request->GetOp())) {
        case Ops::kOpen: {
            Open(qp, reinterpret_cast<generic_posix_open_request*>(request), creds);
            return;
        }
        case Ops::kClose: {
            Close(qp, reinterpret_cast<generic_posix_close_request*>(request), creds);
            return;
        }
        case Ops::kInit:
        case Ops::kWrite:
        case Ops::kRead:
        case Ops::kIOComplete: {
            Passthrough(qp, reinterpret_cast<generic_posix_passthrough_request*>(request), creds);
            return;
        }
    }
}

int labstor::GenericPosix::Server::PriorSlash(char *path, int len) {
    int i = 0;
    for(i = len - 1; i >= 0; --i) {
       if(path[i] == '/') {
           return i;
       }
    }
    return 0;
}

void labstor::GenericPosix::Server::Open(labstor::ipc::queue_pair *qp, generic_posix_open_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE("")
    char *path = client_rq->path_;
    int len = strlen(path);
    int fd = client_rq->GetFD();
    uint32_t ns_id;
    uint64_t pid_fd = PID_FD(creds->pid_, fd);
    labstor::Module *module;
    while(len > 0) {
        if(namespace_->GetIfExists(labstor::ipc::string(client_rq->path_, len), ns_id)) {
            module = namespace_->Get(ns_id);
            module->ProcessRequest(qp, reinterpret_cast<labstor::ipc::request*>(client_rq), creds);
            fd_to_ns_id_.Set(pid_fd, ns_id);
            client_rq->Complete(LABSTOR_REQUEST_SUCCESS);
            qp->Complete(client_rq);
            return;
        }
        len = PriorSlash(path, len);
    }
    client_rq->Complete(LABSTOR_GENERIC_FS_PATH_NOT_FOUND);
    qp->Complete(client_rq);
}

void labstor::GenericPosix::Server::Close(labstor::ipc::queue_pair *qp, generic_posix_close_request *client_rq, labstor::credentials *creds) {
    labstor::Module *module;
    uint64_t pid_fd = PID_FD(creds->pid_, client_rq->GetFD());
    uint32_t ns_id;
    if(fd_to_ns_id_.Find(pid_fd, ns_id)) {
        fd_to_ns_id_.Remove(ns_id);
        module = namespace_->Get(ns_id);
        module->ProcessRequest(qp, reinterpret_cast<labstor::ipc::request*>(client_rq), creds);
        client_rq->Complete(LABSTOR_GENERIC_FS_SUCCESS);
        qp->Complete(client_rq);
        return;
    }
    client_rq->Complete(LABSTOR_GENERIC_FS_PATH_NOT_FOUND);
    qp->Complete(client_rq);
}

void labstor::GenericPosix::Server::Passthrough(labstor::ipc::queue_pair *qp, generic_posix_passthrough_request *client_rq, labstor::credentials *creds) {
    labstor::Module *module;
    uint64_t pid_fd = PID_FD(creds->pid_, client_rq->GetFD());
    uint32_t ns_id;
    if(fd_to_ns_id_.Find(pid_fd, ns_id)) {
        module = namespace_->Get(ns_id);
        module->ProcessRequest(qp, reinterpret_cast<labstor::ipc::request*>(client_rq), creds);
        return;
    }
    client_rq->Complete(LABSTOR_GENERIC_FS_INVALID_FD);
    qp->Complete(client_rq);

}

LABSTOR_MODULE_CONSTRUCT(labstor::GenericPosix::Server, GENERIC_POSIX_MODULE_ID)