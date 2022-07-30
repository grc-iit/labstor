
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <labmods/generic_posix/generic_posix.h>
#include <labmods/lru/client/lru_client.h>

void labstor::LRU::Client::Register(YAML::Node config) {
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(LABFS_MODULE_ID, config["labmod_uuid"].as<std::string>());
    LABSTOR_REGISTRAR->InitializeInstance<register_request>(ns_id_, config["next"].as<std::string>());
    if(config["do_format"].as<bool>()) {
        labstor::GenericBlock::Client *block_dev = namespace_->LoadClientModule<labstor::GenericBlock::Client>(config["device"]);
        if(block_dev == nullptr) {
            throw NOT_YET_IMPLEMENTED.format();
        }
        void *buf = calloc(SMALL_BLOCK_SIZE, 0);
        block_dev->Write(buf, SMALL_BLOCK_SIZE, 0);
    }
}

int labstor::LRU::Client::Open(int fd, const char *path, int pathlen, int oflag) {
    AUTO_TRACE("")
    labstor::GenericPosix::open_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    client_rq = ipc_manager_->AllocRequest<labstor::GenericPosix::open_request>(qp);
    client_rq->ClientInit(ns_id_, path, oflag, fd);

    //Complete CLIENT -> SERVER interaction
    qp->Enqueue<labstor::GenericPosix::open_request>(client_rq, qtok);
    client_rq = ipc_manager_->Wait<labstor::GenericPosix::open_request>(qtok);
    if(client_rq->GetCode() == LABSTOR_GENERIC_FS_PATH_NOT_FOUND) {
        fd = LABSTOR_GENERIC_FS_PATH_NOT_FOUND;
    }

    //Free requests
    ipc_manager_->FreeRequest<labstor::GenericPosix::open_request>(qtok, client_rq);
    return fd;
}

int labstor::LRU::Client::Close(int fd) {
    AUTO_TRACE("")
    labstor::GenericPosix::close_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    int code;

    //Get SERVER QP
    ipc_manager_->GetQueuePair(qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_PRIMARY | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY);

    //Create CLIENT -> SERVER message
    client_rq = ipc_manager_->AllocRequest<labstor::GenericPosix::close_request>(qp);
    client_rq->Start(ns_id_, fd);

    //Complete CLIENT -> SERVER interaction
    qp->Enqueue<labstor::GenericPosix::close_request>(client_rq, qtok);
    client_rq = ipc_manager_->Wait<labstor::GenericPosix::close_request>(qtok);
    code = client_rq->GetCode();

    //Free requests
    ipc_manager_->FreeRequest<labstor::GenericPosix::close_request>(qtok, client_rq);

    return code;
}

labstor::ipc::qtok_t labstor::LRU::Client::AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size) {
    AUTO_TRACE("")
    labstor::GenericPosix::io_request *client_rq;
    labstor::queue_pair *qp;
    labstor::ipc::qtok_t qtok;
    ssize_t ret;

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

ssize_t labstor::LRU::Client::IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size) {
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

LABSTOR_MODULE_CONSTRUCT(labstor::LRU::Client, LABFS_MODULE_ID)