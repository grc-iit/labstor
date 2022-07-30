
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

#ifndef LABSTOR_REQUEST_KERNEL_H
#define LABSTOR_REQUEST_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#ifdef __cplusplus
#include <cstring>
#endif

struct labstor_request {
    labstor_req_id_t req_id_;
    uint32_t ns_id_;
    uint32_t code_;
    uint16_t op_;
#ifdef __cplusplus
    inline labstor_request() = default;
    inline void Start(uint32_t req_id, uint32_t ns_id, uint16_t op, uint32_t code) {
        req_id_ = req_id;
        ns_id_ = ns_id;
        op_ = op;
        code_ = code;
    }

    inline uint32_t GetNamespaceID() { return ns_id_; }
    inline uint32_t GetCode() { return code_; }
    inline uint32_t GetRequestID() { return req_id_; }
    inline uint16_t GetOp() { return op_; }

    inline void SetNamespaceID(uint32_t ns_id) {  ns_id_ = ns_id; }
    inline void SetCode(uint32_t code) { code_ = code; }
    inline void SetRequestID(uint32_t req_id) { req_id_ = req_id; }
    inline void SetOp(uint32_t op) { op_ = op; }
#endif
};



#ifdef __cplusplus

namespace labstor::ipc {
    typedef labstor_request request;
}

#endif

#endif //LABSTOR_REQUEST_KERNEL_H