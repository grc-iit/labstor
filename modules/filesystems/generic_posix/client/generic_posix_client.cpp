//
// Created by lukemartinlogan on 8/20/21.
//

#include <registrar/client/registrar_client.h>
#include <labstor/userspace/client/ipc_manager.h>
#include "generic_posix_client.h"
#include <mutex>

void labstor::GenericPosix::Client::Initialize() {
    AUTO_TRACE("")
    //Retreive the namespace ID from the server
    if(lock_.try_lock()) {
        if(!is_initialized_) {
            TRACEPOINT("HERE", (size_t)this)
            ipc_manager_->Connect();
            labstor::Registrar::Client registrar;
            ns_id_ = registrar.GetNamespaceID(GENERIC_POSIX_MODULE_ID);
            is_initialized_ = true;
            fd_min_ = LABSTOR_FD_MIN; //TODO: Should be queried from the server
        }
    }
}

int labstor::GenericPosix::Client::Open(const char *path, int oflag) {
    AUTO_TRACE("")
    generic_posix_open_request *client_rq;
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    int fd;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Allocate an fd
    fd = AllocateFD();

    //Create CLIENT -> SERVER message
    client_rq = ipc_manager_->AllocRequest<generic_posix_open_request>(qp);
    client_rq->Start(ns_id_, path, oflag);

    //Complete CLIENT -> SERVER interaction
    qp->Enqueue<generic_posix_open_request>(client_rq, qtok);
    client_rq = ipc_manager_->Wait<generic_posix_open_request>(qtok);
    fd = client_rq->GetFD();

    //Free requests
    ipc_manager_->FreeRequest<generic_posix_open_request>(qtok, client_rq);

    return fd;
}

int labstor::GenericPosix::Client::Close(int fd) {
    AUTO_TRACE("")
    generic_posix_close_request *client_rq;
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    int code;
    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Deallocate FD
    FreeFD(fd);

    //Create CLIENT -> SERVER message
    client_rq = ipc_manager_->AllocRequest<generic_posix_close_request>(qp);
    client_rq->Start(ns_id_, fd);

    //Complete CLIENT -> SERVER interaction
    qp->Enqueue<generic_posix_close_request>(client_rq, qtok);
    client_rq = ipc_manager_->Wait<generic_posix_close_request>(qtok);
    code = client_rq->GetCode();

    //Free requests
    ipc_manager_->FreeRequest<generic_posix_close_request>(qtok, client_rq);

    return code;
}

labstor::ipc::qtok_t labstor::GenericPosix::Client::IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size) {
    AUTO_TRACE("")
    generic_posix_io_request *client_rq;
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    ssize_t ret;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    client_rq = ipc_manager_->AllocRequest<generic_posix_io_request>(qp);
    client_rq->Start(ns_id_, op, fd, buf, size);

    //Enqueue the message
    qp->Enqueue<generic_posix_io_request>(client_rq, qtok);
    return qtok;
}

ssize_t labstor::GenericPosix::Client::IOSync(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size) {
    AUTO_TRACE("")
    generic_posix_io_request *client_rq;
    labstor::ipc::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    ssize_t ret;
    qtok = IO(op, fd, buf, size);
    client_rq = ipc_manager_->Wait<generic_posix_io_request>(qtok);
    ret = client_rq->GetSize();
    ipc_manager_->FreeRequest<generic_posix_io_request>(qtok, client_rq);
    return ret;
}

LABSTOR_MODULE_CONSTRUCT(labstor::GenericPosix::Client, GENERIC_POSIX_MODULE_ID);