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

    //Load head table from disk

    //Load all
}
inline void labstor::LabFS::Server::Open(labstor::queue_pair *qp, labstor::GenericPosix::open_request *client_rq, labstor::credentials *creds) {
    //Allocate file UUID
    //Map file path to UUID
    //Append to log
}
inline void labstor::LabFS::Server::Close(labstor::queue_pair *qp, labstor::GenericPosix::close_request *client_rq, labstor::credentials *creds) {
    //sync all data & metadata back to storage
}
inline void labstor::LabFS::Server::IOStart(labstor::queue_pair *qp, labstor::GenericPosix::io_request *client_rq, labstor::credentials *creds) {
    size_t total_io = client_rq->size_;
    for(size_t cur_io = 0; cur_io < total_io; ) {
        //Allocate a block from the device
        //Create request designated to the next module
        //Send block request to the next module
        //Append (IO_START, file_off, block)
    }
}
inline void labstor::LabFS::Server::IOComplete(labstor::queue_pair *qp, labstor::GenericPosix::io_request *client_rq, labstor::credentials *creds) {
    //Append IO_END block request to log
}

LABSTOR_MODULE_CONSTRUCT(labstor::LabFS::Server, LABFS_MODULE_ID)