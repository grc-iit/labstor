
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
#include <labmods/block_fs/block_fs.h>
#include <labmods/block_fs/client/block_fs_client.h>

void labstor::BlockFS::Client::Register(YAML::Node config) {
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(BLOCKFS_MODULE_ID, config["labmod_uuid"].as<std::string>());
    LABSTOR_REGISTRAR->InitializeInstance<register_request>(ns_id_, config["next"].as<std::string>());
}

int labstor::BlockFS::Client::Open(int fd, const char *path, int pathlen, int oflag) {
    AUTO_TRACE("")
    fd_to_file_.emplace(fd, labstor::GenericPosix::FILE());
    return fd;
}

int labstor::BlockFS::Client::Close(int fd) {
    AUTO_TRACE("")
    fd_to_file_.erase(fd);
    return 0;
}

labstor::ipc::qtok_t labstor::BlockFS::Client::AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size) {
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
    client_rq->Start(ns_id_, op, fd, buf, off, size);

    //Enqueue the message
    qp->Enqueue<labstor::GenericPosix::io_request>(client_rq, qtok);
    return qtok;
}

labstor::ipc::qtok_t labstor::BlockFS::Client::AIO(labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size) {
    AUTO_TRACE("")
    labstor::ipc::qtok_t qtok;
    labstor::GenericPosix::FILE &fp = fd_to_file_[fd];
    qtok = AIO(op, fd, buf, fp.off_, size);
    fp.off_ += size;
    return qtok;
}

ssize_t labstor::BlockFS::Client::IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size) {
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

ssize_t labstor::BlockFS::Client::IO(labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size) {
    AUTO_TRACE("")
    labstor::ipc::qtok_t qtok;
    labstor::GenericPosix::FILE &fp = fd_to_file_[fd];
    ssize_t ret = IO(op, fd, buf, fp.off_, size);
    fp.off_ += ret;
    return ret;
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlockFS::Client, BLOCKFS_MODULE_ID)