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

/*
 * We do not need qtokens to index queues; they are just ids.
 * We need qids to be unique.
 * Qids should be allocated using a combination
 *
 * Queue Index Properties:
 * [flags | hash(str, runtime_id)] #For keying things to same file
 * [flags | core] #For scalable, low-latency request submission
 * [flags | depth | refcnt] #For batch submissions
 * */

namespace labstor::Client {

class IPCManager {
private:
    UnixSocket serversock_;
    labstor::GenericAllocator *shmem_alloc_;
    labstor::GenericAllocator *internal_alloc_;
    std::vector<labstor::ipc::queue_pair> shmem_qps_;
    std::vector<labstor::ipc::queue_pair> internal_qps_;
public:
    void Connect(int num_queues);
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, bool shmem) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(shmem) {
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, sched_getcpu(), shmem_qps_.size(), 0)];
            } else {
                qp = internal_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, sched_getcpu(), internal_qps_.size(), 1)];
            }
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, int hash, bool shmem) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(shmem) {
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, hash, shmem_qps_.size(), 0)];
            } else {
                qp = internal_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, hash, internal_qps_.size(), 1)];
            }
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, const std::string &str, uint32_t ns_id, bool shmem) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            if(shmem) {
                qp = shmem_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, str, ns_id, shmem_qps_.size(), 0)];
            } else {
                qp = internal_qps_[labstor::ipc::queue_pair::GetStreamQueuePairOff(flags, str, ns_id, shmem_qps_.size(), 0)];
            }
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, uint32_t depth=0) {
        if(LABSTOR_QP_IS_BATCH(flags)) {
            uint32_t sq_sz = labstor::ipc::request_queue::GetSize(depth);
            uint32_t cq_sz = labstor::ipc::request_map::GetSize(depth);
            qp.sq.Init(internal_alloc_->Alloc(sq_sz), sq_sz, flags);
            qp.cq.Init(internal_alloc_->Alloc(cq_sz), cq_sz);
            return;
        }
    }
    void PauseQueues();
    void WaitForPause();
    void ResumeQueues();
private:
    void CreateQueuesSHMEM(int num_queues);
    void CreateInternalQueues(int num_queues);
};

}
#endif //LABSTOR_CLIENT_IPCMANAGER_H
