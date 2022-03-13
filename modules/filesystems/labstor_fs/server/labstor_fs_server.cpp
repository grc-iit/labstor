//
// Created by lukemartinlogan on 8/20/21.
//

#include <modules/filesystems/generic_posix/generic_posix.h>
#include <modules/filesystems/labstor_fs/labstor_fs.h>
#include <modules/filesystems/labstor_fs/server/labstor_fs_server.h>

void labstor::LabFS::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch(static_cast<labstor::GenericPosix::Ops>(request->GetOp())) {
        case labstor::GenericPosix::Ops::kOpen: {
            Open(qp, reinterpret_cast<labstor::GenericPosix::open_request*>(request), creds);
            return;
        }
        case labstor::GenericPosix::Ops::kClose: {
            Close(qp, reinterpret_cast<labstor::GenericPosix::close_request*>(request), creds);
            return;
        }
        case labstor::GenericPosix::Ops::kWrite:
        case labstor::GenericPosix::Ops::kRead: {
            IOStart(qp, reinterpret_cast<labstor::GenericPosix::io_request*>(request), creds);
            return;
        }
        case labstor::GenericPosix::Ops::kIOComplete: {
            IOComplete(qp, reinterpret_cast<labstor::GenericPosix::io_request*>(request), creds);
            return;
        }
    }
}
inline void labstor::LabFS::Server::Initialize(labstor::ipc::request *rq) {
    labstor::Module *module;
    register_request *reg_rq = reinterpret_cast<register_request*>(rq);
    next_module_ = namespace_->Get(reg_rq->next_);
}
inline void labstor::LabFS::Server::Open(labstor::queue_pair *qp, labstor::GenericPosix::open_request *client_rq, labstor::credentials *creds) {
    //Get INODE by fd
    //Complete the I/O request
}
inline void labstor::LabFS::Server::Close(labstor::queue_pair *qp, labstor::GenericPosix::close_request *client_rq, labstor::credentials *creds) {
    //Unmark inode as busy
    //Complete the I/O request
}
inline void labstor::LabFS::Server::IOStart(labstor::queue_pair *qp, labstor::GenericPosix::io_request *client_rq, labstor::credentials *creds) {
    //Allocate free blocks on the device (per-core ring buffer of 4KB pages)
    //Store the mappings
}
inline void labstor::LabFS::Server::IOComplete(labstor::queue_pair *qp, labstor::GenericPosix::io_request *client_rq, labstor::credentials *creds) {
}

LABSTOR_MODULE_CONSTRUCT(labstor::LabFS::Server, LABFS_MODULE_ID)