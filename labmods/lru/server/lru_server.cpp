//
// Created by lukemartinlogan on 8/20/21.
//

#include <labmods/generic_posix/generic_posix.h>
#include <labmods/lru/server/lru_server.h>
#include <list>

bool labstor::LRU::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch(static_cast<labstor::GenericPosix::Ops>(request->GetOp())) {
        case labstor::GenericPosix::Ops::kInit: {
            return Initialize(qp, request, creds);
        }
        case labstor::GenericPosix::Ops::kOpen: {
            return Open(qp, reinterpret_cast<labstor::GenericPosix::open_request*>(request), creds);
        }
        case labstor::GenericPosix::Ops::kClose: {
            return Close(qp, reinterpret_cast<labstor::GenericPosix::close_request*>(request), creds);
        }
        case labstor::GenericPosix::Ops::kWrite:
        case labstor::GenericPosix::Ops::kRead: {
            return IO(qp, reinterpret_cast<labstor::GenericPosix::io_request*>(request), creds);
        }
    }
    return true;
}
inline bool labstor::LRU::Server::Initialize(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
}
inline bool labstor::LRU::Server::Open(labstor::queue_pair *qp, labstor::GenericPosix::open_request *client_rq, labstor::credentials *creds) {
}
inline bool labstor::LRU::Server::Close(labstor::queue_pair *qp, labstor::GenericPosix::close_request *client_rq, labstor::credentials *creds) {
}
inline bool labstor::LRU::Server::IO(labstor::queue_pair *qp, labstor::GenericPosix::io_request *client_rq, labstor::credentials *creds) {
}

LABSTOR_MODULE_CONSTRUCT(labstor::LRU::Server, LABFS_MODULE_ID)