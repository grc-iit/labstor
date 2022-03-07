//
// Created by lukemartinlogan on 8/20/21.
//

#include <generic_posix/generic_posix.h>
#include <labstor_fs/labstor_fs.h>
#include <labstor_fs/client/labstor_fs_client.h>

void labstor::LabFS::Client::Init(uint32_t ns_id, char *iosched_mount) {
    init_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    int fd;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    client_rq = ipc_manager_->AllocRequest<init_request>(qp);
    client_rq->Start(ns_id, iosched_mount);

    //Complete CLIENT -> SERVER interaction
    qp->Enqueue<init_request>(client_rq, qtok);
    client_rq = ipc_manager_->Wait<init_request>(qtok);

    //Free requests
    ipc_manager_->FreeRequest<init_request>(qtok, client_rq);
}

LABSTOR_MODULE_CONSTRUCT()