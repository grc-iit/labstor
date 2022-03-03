//
// Created by lukemartinlogan on 3/3/22.
//

#ifndef LABSTOR_QUEUE_PAIR_H
#define LABSTOR_QUEUE_PAIR_H

#include "shmem_qtok.h"

namespace labstor {
class queue_pair {
private:
    labstor::ipc::qid_t qid_;
public:
    labstor::ipc::qid_t GetQID() {
        return qid_;
    }

    virtual uint32_t GetDepth() = 0;
    virtual int GetPID() = 0;

    template<typename T>
    inline bool Enqueue(T *rq, labstor::ipc::qtok_t &qtok) {
        return _Enqueue(reinterpret_cast<void*>(rq), qtok);
    }
    template<typename T>
    inline bool Dequeue(T *&rq) {
        bool did_dequeue;
        rq = reinterpret_cast<T*>(_Dequeue(did_dequeue));
        return did_dequeue;
    }
    template<typename S, typename T=S>
    inline void Complete(S *old_rq, T *new_rq) {
        _Complete(reinterpret_cast<void*>(old_rq), reinterpret_cast<void*>(new_rq));
    }
    template<typename T>
    inline void Complete(T *rq) {
        _Complete(reinterpret_cast<void*>(rq));
    }
    template<typename T>
    inline void Complete(labstor::ipc::qtok_t &qtok, T *rq) {
        _Complete(qtok, reinterpret_cast<void*>(rq));
    }
    template<typename T>
    inline bool IsComplete(labstor::ipc::qtok_t &qtok, T *&rq) {
        bool did_dequeue;
        rq = reinterpret_cast<T*>(_IsComplete(did_dequeue));
        return did_dequeue;
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

private:
    inline virtual bool _Enqueue(void *rq, labstor::ipc::qtok_t &qtok) = 0;
    inline virtual void* _Dequeue(bool did_dequeue) = 0;
    inline virtual void _Complete(void *old_rq, void *new_rq) = 0;
    inline virtual void _Complete(void *rq) = 0;
    inline virtual void _Complete(labstor::ipc::qtok_t &qtok, void *rq) = 0;
    inline virtual bool _IsComplete(labstor::ipc::qtok_t &qtok, void *&rq) = 0;
    inline virtual void* _Wait(uint32_t req_id) = 0;
    inline virtual void* _Wait(uint32_t req_id, uint32_t max_ms) = 0;
    inline void* _Wait(labstor::ipc::qtok_t &qtok) {
        return _Wait(qtok.req_id_);
    }
    inline void* _Wait(labstor::ipc::qtok_t &qtok, uint32_t max_ms) {
        return _Wait(qtok.req_id_, max_ms);
    }
};
}

#endif //LABSTOR_QUEUE_PAIR_H
