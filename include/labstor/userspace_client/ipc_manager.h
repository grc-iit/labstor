//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_CLIENT_IPCMANAGER_H
#define LABSTOR_CLIENT_IPCMANAGER_H


#include <vector>
#include <labstor/types/socket.h>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

namespace labstor::Client {

class IPCManager {
private:
    int pid_;
    UnixSocket serversock_;
    labstor::GenericAllocator *shmem_alloc_;
    labstor::GenericAllocator *private_alloc_;
    std::vector<labstor::ipc::queue_pair> shmem_qps_;
    std::vector<labstor::ipc::queue_pair> private_qps_;
public:
    void Connect();
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(LABSTOR_QP_IS_SHMEM(flags)) {
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, sched_getcpu(), shmem_qps_.size(), 0)];
            } else {
                qp = private_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, sched_getcpu(), private_qps_.size(), 1)];
            }
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, int hash) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(LABSTOR_QP_IS_SHMEM(flags)) {
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, hash, shmem_qps_.size(), 0)];
            } else {
                qp = private_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, hash, private_qps_.size(), 1)];
            }
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, const std::string &str, uint32_t ns_id) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(LABSTOR_QP_IS_SHMEM(flags)) {
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, str, ns_id, shmem_qps_.size(), 0)];
            } else {
                qp = private_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, str, ns_id, shmem_qps_.size(), 0)];
            }
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, labstor::ipc::qtok_t &qtok) {
        if(LABSTOR_QP_IS_SHMEM(qtok.qid)) {
            qp = shmem_qps_[LABSTOR_GET_QP_IDX(qtok.qid)];
        } else {
            qp = private_qps_[LABSTOR_GET_QP_IDX(qtok.qid)];
        }
    }
    void Wait(labstor::ipc::qtok_t &qtok) {
        labstor::ipc::request *rq;
        labstor::ipc::queue_pair qp;
        GetQueuePair(qp, qtok);
        rq = qp.Wait(qtok.req_id);

    }
    void Wait(labstor::ipc::qtok_set &qtoks) {
        for(int i = 0; i < qtoks.GetLength(); ++i) {
            Wait(qtoks[i]);
        }
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
