
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

#ifndef LABSTOR_POSIX_CLIENT_H
#define LABSTOR_POSIX_CLIENT_H

#include <labmods/generic_posix/generic_posix.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>
#include <labstor/userspace/util/error.h>
#include <mutex>

namespace labstor::Posix {

class Client : public labstor::Module {
public:
    explicit Client(labstor::id &&module_id) : labstor::Module(module_id) {}
    virtual int Open(int fd, const char *path, int pathlen, int oflag) = 0;
    virtual int Close(int fd) = 0;
    virtual labstor::ipc::qtok_t AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size) = 0;
    virtual labstor::ipc::qtok_t AIO(labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size) = 0;
    virtual ssize_t IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size) = 0;
    virtual ssize_t IO(labstor::GenericPosix::Ops op, int fd, void *buf, ssize_t size) = 0;
};

}

#endif //LABSTOR_POSIX_CLIENT_H