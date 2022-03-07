//
// Created by lukemartinlogan on 8/20/21.
//

#include <labstor_fs.h>
#include <labstor_fs/server/labstor_fs_server.h>
#include <generic_posix/generic_posix.h>

void labstor::LabFS::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch(static_cast<labstor::GenericPosix::Ops>(request->GetOp())) {
        case labstor::GenericPosix::Ops::kInit: {
            Init(qp, reinterpret_cast<init_request*>(request), creds);
            return;
        }
        case labstor::GenericPosix::Ops::kOpen: {
            Open(qp, reinterpret_cast<generic_posix_open_request*>(request), creds);
            return;
        }
        case labstor::GenericPosix::Ops::kClose: {
            Close(qp, reinterpret_cast<generic_posix_close_request*>(request), creds);
            return;
        }
        case labstor::GenericPosix::Ops::kWrite:
        case labstor::GenericPosix::Ops::kRead: {
            IOStart(qp, reinterpret_cast<generic_posix_io_request*>(request), creds);
            return;
        }
        case labstor::GenericPosix::Ops::kIOComplete: {
            IOComplete(qp, reinterpret_cast<generic_posix_io_request*>(request), creds);
            return;
        }
    }
}
inline void labstor::LabFS::Server::Init(labstor::queue_pair *qp, init_request *client_rq, labstor::credentials *creds) {
    AUTO_TRACE(client_rq->iosched_mount_)
    labstor::Module *module;
    uint32_t ns_id;
    //TODO: Initialize inode cache
        //INODE: 0/home -> ID: 1
        //INODE: 1/luke -> ID: 2
        //INODE: 2/hi.tx -> ID: 3
    //TODO: Initialize block allocator
    //Find the IOScheduler to send requests to
    if(namespace_->GetIfExists(labstor::ipc::string(client_rq->iosched_mount_), ns_id)) {
        module = namespace_->Get(ns_id);
        module->ProcessRequest(qp, reinterpret_cast<labstor::ipc::request*>(client_rq), creds);
        client_rq->Complete(LABSTOR_GENERIC_FS_SUCCESS);
    } else {
        client_rq->Complete(LABSTOR_GENERIC_FS_PATH_NOT_FOUND);
    }
    //Finish request
    qp->Complete(client_rq);
}
inline void labstor::LabFS::Server::Open(labstor::queue_pair *qp, generic_posix_open_request *client_rq, labstor::credentials *creds) {
    //Determine if INODE exists
    //Complete the I/O request
}
inline void labstor::LabFS::Server::Close(labstor::queue_pair *qp, generic_posix_close_request *client_rq, labstor::credentials *creds) {
    //Unmark inode as busy
    //Complete the I/O request
}
inline void labstor::LabFS::Server::IOStart(labstor::queue_pair *qp, generic_posix_io_request *client_rq, labstor::credentials *creds) {
    //Allocate free blocks on the device (per-core ring buffer of 4KB pages)
    //Store the mappings
}
inline void labstor::LabFS::Server::IOComplete(labstor::queue_pair *qp, generic_posix_io_request *client_rq, labstor::credentials *creds) {
}