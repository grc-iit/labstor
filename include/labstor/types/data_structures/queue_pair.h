
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

#ifndef LABSTOR_QUEUE_PAIR2_H
#define LABSTOR_QUEUE_PAIR2_H

#include "shmem_qtok.h"
#include "shmem_request.h"
#include "labstor/constants/busy_wait.h"

#ifdef __cplusplus

namespace labstor {
class queue_pair {
public:
    int GetPID() {
        return GetQID().pid_;
    }

    inline virtual labstor::ipc::qid_t& GetQID() = 0;
    inline virtual uint32_t GetDepth() = 0;

    template<typename T>
    inline bool Enqueue(T *rq, labstor::ipc::qtok_t &qtok) {
        return _Enqueue(reinterpret_cast<labstor::ipc::request*>(rq), qtok);
    }
    template<typename T>
    inline bool Peek(T *&rq, int i) {
        return _Peek(reinterpret_cast<labstor::ipc::request**>(&rq), i);
    }
    template<typename T>
    inline bool Dequeue(T *&rq) {
        return _Dequeue(reinterpret_cast<labstor::ipc::request**>(&rq));
    }
    template<typename S, typename T=S>
    inline void Complete(S *old_rq, T *new_rq) {
        _Complete(old_rq, new_rq);
    }
    template<typename T>
    inline void Complete(T *rq) {
        _Complete(reinterpret_cast<labstor::ipc::request*>(rq));
    }
    template<typename T>
    inline void Complete(labstor::ipc::qtok_t &qtok, T *rq) {
        _Complete(qtok, reinterpret_cast<labstor::ipc::request*>(rq));
    }
    template<typename T>
    inline bool IsComplete(int req_id, T *&rq) {
        return _IsComplete(req_id, reinterpret_cast<labstor::ipc::request**>(&rq));
    }
    template<typename T>
    inline bool IsComplete(labstor::ipc::qtok_t &qtok, T *&rq) {
        return _IsComplete(qtok.req_id_, reinterpret_cast<labstor::ipc::request**>(&rq));
    }
    template<typename T>
    inline T* Wait(uint32_t req_id) {
        return reinterpret_cast<T*>(_Wait(req_id));
    }
    template<typename T>
    inline T* Wait(uint32_t req_id, uint32_t max_ms) {
        return reinterpret_cast<T*>(_Wait(req_id, max_ms));
    }
    template<typename T>
    inline T* Wait(labstor::ipc::qtok_t &qtok) {
        return reinterpret_cast<T*>(_Wait(qtok));
    }
    template<typename T>
    inline T* Wait(labstor::ipc::qtok_t &qtok, uint32_t max_ms) {
        return reinterpret_cast<T*>(_Wait(qtok, max_ms));
    }

    static labstor_qid_t GetQID(labstor_qid_type_t type, labstor_qid_flags_t flags, uint32_t hash, uint32_t num_qps, int pid) {
        labstor_qid_t qid;
        qid.type_ = type;
        qid.cnt_ = hash % num_qps;
        qid.pid_ = pid;
        qid.flags_ = flags;
        return qid;
    }
    static labstor_qid_t GetQID(labstor_qid_type_t type, labstor_qid_flags_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
        uint32_t hash = 0;
        for(size_t i = 0; i < str.size(); ++i) {
            hash += str[i] << 4*(i%4);
        }
        hash *= ns_id;
        return GetQID(type, flags, hash, num_qps, pid);
    }
    static uint32_t GetQIDOff(labstor_qid_type_t type, labstor_qid_flags_t flags, uint32_t hash, uint32_t num_qps, int pid) {
        return GetQID(type, flags, hash, num_qps, pid).cnt_;
    }
    static uint32_t GetQIDOff(labstor_qid_type_t type, labstor_qid_flags_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
        return GetQID(type, flags, str, ns_id, num_qps, pid).cnt_;
    }

private:
    inline virtual bool _Enqueue(labstor::ipc::request *rq, labstor::ipc::qtok_t &qtok) = 0;
    inline virtual bool _Peek(labstor::ipc::request **rq, int i) = 0;
    inline virtual bool _Dequeue(labstor::ipc::request **rq) = 0;
    inline virtual void _Complete(labstor_req_id_t req_id, labstor::ipc::request *rq) = 0;
    inline virtual bool _IsComplete(labstor_req_id_t req_id, labstor::ipc::request **rq) = 0;

    inline void _Complete(labstor::ipc::request *old_rq, labstor::ipc::request *new_rq) {
        _Complete(old_rq->req_id_, new_rq);
    }
    inline void _Complete(labstor::ipc::request *rq) {
        _Complete(rq->req_id_, rq);
    }
    inline void _Complete(labstor::ipc::qtok_t &qtok, labstor::ipc::request *rq) {
        _Complete(qtok.req_id_, rq);
    }
    inline labstor::ipc::request* _Wait(uint32_t req_id) {
        LABSTOR_INF_SPINWAIT_PREAMBLE()
        labstor::ipc::request *ret = NULL;
        LABSTOR_INF_SPINWAIT_START()
            if(_IsComplete(req_id, &ret)) {
                return ret;
            }
        LABSTOR_INF_SPINWAIT_END()
    }
    inline labstor::ipc::request* _Wait(uint32_t req_id, uint32_t max_ms) {
        LABSTOR_TIMED_SPINWAIT_PREAMBLE()
        labstor::ipc::request *ret = NULL;
        LABSTOR_TIMED_SPINWAIT_START(max_ms)
            if(_IsComplete(req_id, &ret)) {
                return ret;
            }
        LABSTOR_TIMED_SPINWAIT_END(max_ms)
        return NULL;
    }
    inline labstor::ipc::request* _Wait(labstor::ipc::qtok_t &qtok) {
        return _Wait(qtok.req_id_);
    }
    inline labstor::ipc::request* _Wait(labstor::ipc::qtok_t &qtok, uint32_t max_ms) {
        return _Wait(qtok.req_id_, max_ms);
    }
};

class user_queue_pair : public queue_pair {
private:
    labstor::ipc::qid_t qid_;
public:
    inline labstor::ipc::qid_t& GetQID() {
        return qid_;
    }
protected:
    inline void SetQID(labstor::ipc::qid_t &qid) {
        qid_ = qid;
    }
};
}

#endif

#endif //LABSTOR_QUEUE_PAIR_H