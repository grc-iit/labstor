
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

#ifndef LABSTOR_SHMEM_QTOK_H
#define LABSTOR_SHMEM_QTOK_H

#ifdef __cplusplus
#include <labstor/userspace/util/serializeable.h>
#endif

#define LABSTOR_QTOK_INVALID(qtok) (qtok.req_id==-1 || qtok.qid==-1)

typedef uint8_t labstor_qid_flags_t;
typedef uint8_t labstor_qid_type_t;

typedef struct labstor_qid_t {
    uint8_t flags_;
    uint8_t type_;
    uint16_t cnt_;
    uint32_t pid_;
#ifdef __cplusplus
    labstor_qid_t() = default;
    labstor_qid_t(uint32_t qid) : flags_(0), cnt_(qid), pid_(0) {}
    uint64_t Hash() {
        uint64_t num = 0;
        num += flags_;
        num <<= 8;
        num += type_;
        num <<= 8;
        num += pid_ % (1<<16);
        num <<= 16;
        num += cnt_;
        return num;
    }
    bool operator ==(const labstor_qid_t &qid) {
        return (qid.flags_ == flags_) && (qid.pid_ == pid_) && (qid.cnt_ == cnt_);
    }
#endif
} labstor_qid_t;

typedef uint32_t labstor_req_id_t;
struct labstor_qtok_t {
    labstor_qid_t qid_;
    labstor_req_id_t req_id_;
#ifdef __cplusplus
    labstor_qtok_t() = default;
    void Init(labstor_qid_t qid_, labstor_req_id_t req_id_) {
        qid_ = qid_;
        req_id_ = req_id_;
    }
#endif
};


#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_qid_t qid_t;
    typedef labstor_qtok_t qtok_t;
    typedef labstor_req_id_t req_id_t;
}
#endif

#endif //LABSTOR_SHMEM_QTOK_H