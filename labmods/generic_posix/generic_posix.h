
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

#ifndef LABSTOR_GENERIC_POSIX_H
#define LABSTOR_GENERIC_POSIX_H

#include <cstring>
#include <labstor/types/data_structures/shmem_request.h>
//#include <labstor/types/data_structures/shmem_poll.h>

#define GENERIC_POSIX_MODULE_ID "GenericPosix"

enum GENERIC_FS_RETURN_CODES {
    LABSTOR_GENERIC_FS_SUCCESS = 0,
    LABSTOR_GENERIC_FS_INVALID_FD = -1350,
    LABSTOR_GENERIC_FS_PATH_NOT_FOUND = -1351,
    LABSTOR_GENERIC_FS_INVALID_PATH = -1352,
};

namespace labstor::GenericPosix {
enum class Ops {
    kInit,
    kOpen,
    kClose,
    kRead,
    kWrite,
    kFsync,
    kFdatasync
};

struct FILE {
    int off_;
    FILE() : off_(0) {}
};

struct open_request : public labstor::ipc::request {
    int oflags_;
    int fd_;
    char path_[];
    inline void ClientInit(int ns_id, const char *path, int oflags, int fd) {
        SetNamespaceID(ns_id);
        SetOp(static_cast<int>(labstor::GenericPosix::Ops::kOpen));
        oflags_ = oflags;
        fd_ = fd;
        strcpy(path_, path);
    }
    inline void Complete(int code) {
        SetCode(code);
    }
    inline int GetFD() {
        return fd_;
    }
};

struct close_request : public labstor::ipc::request{
    int fs_ns_id_;
    int fd_;
    inline void Start(int ns_id, int fd) {
        SetNamespaceID(ns_id);
        SetOp(static_cast<int>(labstor::GenericPosix::Ops::kClose));
        fd_ = fd;
    }
    inline void Complete(int ret) {
        SetCode(ret);
    }
    inline void SetFD(int fd) {
        fd_ = fd;
    }
    inline int GetFD() {
        return fd_;
    }
};

struct passthrough_request : public labstor::ipc::request {
    uint32_t fs_ns_id_;
    int fd_;
    inline void SetFD(int fd) {
        fd_ = fd;
    }
    inline int GetFD() {
        return fd_;
    }
    inline void Complete(int code) {
        SetCode(code);
    }
};

struct io_request : passthrough_request {
    void *buf_;
    size_t off_;
    ssize_t size_;
    int num_qtoks_, cur_qtok_;
    labstor::ipc::qtok_t *qtoks_;
    inline void Start(int ns_id, labstor::GenericPosix::Ops op, int fd, void *buf, size_t off, ssize_t size) {
        SetNamespaceID(ns_id);
        SetOp(static_cast<int>(op));
        fd_ = fd;
        buf_ = buf;
        size_ = size;
        off_ = off;
    }
    inline void SetQtoks(int num_qtoks, labstor::ipc::qtok_t *qtoks) {
        num_qtoks_ = num_qtoks;
        qtoks_ = qtoks;
        cur_qtok_ = 0;
    }
    inline void Complete(ssize_t size, int code) {
        size_ = size;
        passthrough_request::Complete(code);
    }
    inline ssize_t GetSize() {
        return size_;
    }
};

int PriorSlash(const char *path, int len) {
    int i = 0;
    for(i = len - 1; i >= 0; --i) {
        if(path[i] == '/') {
            return i;
        }
    }
    return 0;
}

}

#endif //LABSTOR_GENERIC_POSIX_H