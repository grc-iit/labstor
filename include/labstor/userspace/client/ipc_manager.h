//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_CLIENT_IPCMANAGER_H
#define LABSTOR_CLIENT_IPCMANAGER_H


#include <thread>
#include <vector>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/types/socket.h>
#include <labstor/constants/constants.h>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/allocator/segment_allocator.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_qtok_set.h>
#include <labstor/types/thread_local.h>
#include <mutex>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

namespace labstor::Client {

class IPCManager {
private:
    int pid_, n_cpu_;
    UnixSocket serversock_;
    labstor::GenericAllocator *shmem_alloc_;
    labstor::segment_allocator *qp_alloc_;
    labstor::GenericAllocator *private_alloc_;
    std::vector<labstor::ipc::queue_pair*> shmem_qps_;
    std::vector<labstor::ipc::queue_pair*> private_qps_;
    bool is_connected_;
public:
    IPCManager() : is_connected_(false) {
        n_cpu_ = get_nprocs_conf();
    }
    void Connect();
    bool IsConnected() {
        return is_connected_;
    }
    inline int GetPid() {
        return pid_;
    }
    inline int GetNumCPU() {
        return n_cpu_;
    }
    inline void* GetBaseRegion() {
        return shmem_alloc_->GetRegion();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, uint16_t flags) {
        AUTO_TRACE("")
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(LABSTOR_QP_IS_SHMEM(flags)) {
                TRACEPOINT("shmem_qps", shmem_qps_.size())
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, labstor::ThreadLocal::GetTid(), shmem_qps_.size(), 0)];
            } else {
                TRACEPOINT("private_qps", private_qps_.size())
                qp = private_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, labstor::ThreadLocal::GetTid(), private_qps_.size(), 1)];
            }
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, uint16_t flags, int hash) {
        AUTO_TRACE("")
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(LABSTOR_QP_IS_SHMEM(flags)) {
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, hash, shmem_qps_.size(), 0)];
            } else {
                qp = private_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, hash, private_qps_.size(), 1)];
            }
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, uint16_t flags, const std::string &str, uint32_t ns_id) {
        AUTO_TRACE("")
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(LABSTOR_QP_IS_SHMEM(flags)) {
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, str, ns_id, shmem_qps_.size(), 0)];
            } else {
                qp = private_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, str, ns_id, shmem_qps_.size(), 0)];
            }
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qtok_t &qtok) {
        AUTO_TRACE("")
        if(LABSTOR_QP_IS_SHMEM(qtok.qid_.flags_)) {
            qp = shmem_qps_[LABSTOR_GET_QP_IDX(qtok.qid_)];
        } else {
            qp = private_qps_[LABSTOR_GET_QP_IDX(qtok.qid_)];
        }
    }
    template<typename T>
    inline T* AllocRequest(labstor::ipc::qid_t qid, uint32_t size) {
        AUTO_TRACE("")
        if(LABSTOR_QP_IS_SHMEM(qid.flags_)) {
            return reinterpret_cast<T*>(shmem_alloc_->Alloc(size, labstor::ThreadLocal::GetTid()));
        } else {
            return reinterpret_cast<T*>(private_alloc_->Alloc(size, labstor::ThreadLocal::GetTid()));
        }
    }
    template<typename T>
    inline T* AllocRequest(labstor::ipc::queue_pair *qp, uint32_t size) {
        return reinterpret_cast<T*>(AllocRequest<T>(qp->GetQid(), size));
    }
    template<typename T>
    inline T* AllocRequest(labstor::ipc::queue_pair *qp) {
        AUTO_TRACE("")
        return reinterpret_cast<T*>(AllocRequest<T>(qp->GetQid(), sizeof(T)));
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qid_t qid, T *rq) {
        AUTO_TRACE("")
        if(LABSTOR_QP_IS_SHMEM(qid.flags_)) {
            shmem_alloc_->Free(reinterpret_cast<void*>(rq));
        } else {
            private_alloc_->Free(reinterpret_cast<void*>(rq));
        }
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qtok_t &qtok, T *rq) {
        AUTO_TRACE("")
        return FreeRequest(qtok.qid_, rq);
    }
    template<typename T=labstor::ipc::request>
    T* Wait(labstor::ipc::qtok_t &qtok) {
        AUTO_TRACE("")
        T *rq;
        labstor::ipc::queue_pair *qp;
        GetQueuePair(qp, qtok);
        rq = qp->Wait<T>(qtok.req_id_);
        return rq;
    }
    template<typename T=labstor::ipc::request>
    int Wait(labstor::ipc::qtok_set &qtoks) {
        AUTO_TRACE("")
        labstor::ipc::qtok_t qtok;
        while(qtoks.Dequeue(qtok)) {
            T *rq = Wait<T>(qtok);
            //TODO: Check if request successful
            FreeRequest(qtok, rq);
        }
        return LABSTOR_REQUEST_SUCCESS;
    }

    template<typename T=labstor::ipc::request>
    int Wait(labstor::ipc::qtok_t *qtoks, int num_qtoks) {
        AUTO_TRACE("")
        labstor::ipc::qtok_t qtok;
        for(int i = 0; i < num_qtoks; ++i) {
            T *rq = Wait<T>(qtoks[i]);
            //TODO: Check if request successful
            FreeRequest(qtok, rq);
        }
        return LABSTOR_REQUEST_SUCCESS;
    }

    void PauseQueues();
    void WaitForPause();
    void ResumeQueues();
private:
    void CreateQueuesSHMEM(int num_queues, int queue_size);
    void CreatePrivateQueues(int num_queues, int queue_size);
};

}
#endif //LABSTOR_CLIENT_IPCMANAGER_H
