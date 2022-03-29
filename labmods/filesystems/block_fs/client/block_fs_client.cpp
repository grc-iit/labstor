//
// Created by lukemartinlogan on 8/20/21.
//

#include <labmods/filesystems/generic_posix/generic_posix.h>
#include <labmods/filesystems/block_fs/block_fs.h>
#include <labmods/filesystems/block_fs/client/block_fs_client.h>

void labstor::BlockFS::Client::Register(char *ns_key, char *next_module) {
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance<register_request>(BLOCKFS_MODULE_ID, ns_key, next_module);
}

int labstor::BlockFS::Client::Open(int fd, const char *path, int pathlen, int oflag) {
    AUTO_TRACE("")
    return fd;
}

int labstor::BlockFS::Client::Close(int fd) {
    AUTO_TRACE("")
    fd_to_file_.erase(fd);
}

labstor::ipc::qtok_t labstor::BlockFS::Client::AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size) {
    AUTO_TRACE("")
    labstor::GenericPosix::io_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    ssize_t ret;

    //Get file offset
    labstor::GenericPosix::FILE *fp = fd_to_file_[fd];

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    client_rq = ipc_manager_->AllocRequest<labstor::GenericPosix::io_request>(qp);
    client_rq->Start(ns_id_, op, fd, buf, size);

    //Enqueue the message
    qp->Enqueue<labstor::GenericPosix::io_request>(client_rq, qtok);
    return qtok;
}

ssize_t labstor::BlockFS::Client::IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size) {
    AUTO_TRACE("")
    labstor::GenericPosix::io_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    ssize_t ret;
    qtok = AIO(op, fd, buf, size);
    client_rq = ipc_manager_->Wait<labstor::GenericPosix::io_request>(qtok);
    ret = client_rq->GetSize();
    ipc_manager_->FreeRequest<labstor::GenericPosix::io_request>(qtok, client_rq);
    return ret;
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlockFS::Client, BLOCKFS_MODULE_ID)