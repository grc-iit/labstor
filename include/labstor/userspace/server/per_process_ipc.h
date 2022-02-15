//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_PER_PROCESS_IPC_H
#define LABSTOR_PER_PROCESS_IPC_H

#include <labstor/types/allocator/allocator.h>
#include <labstor/types/allocator/segment_allocator.h>
#include <vector>

namespace labstor::Server {
struct PerProcessIPC {
    UnixSocket clisock_;
    labstor::credentials creds_;
    int region_id_;
    labstor::GenericAllocator *alloc_;
    labstor::segment_allocator *qp_alloc_;
    int num_stream_qps_;
    std::vector<std::vector<labstor::ipc::queue_pair*>> qps_;

    PerProcessIPC() : num_stream_qps_(0) {}

    PerProcessIPC(int fd, labstor::credentials creds) : clisock_(fd), creds_(creds), num_stream_qps_(0) {}

    PerProcessIPC(const PerProcessIPC &old_ipc) {
        clisock_ = old_ipc.clisock_;
        creds_ = old_ipc.creds_;
        alloc_ = old_ipc.alloc_;
        num_stream_qps_ = old_ipc.num_stream_qps_;
    }

    inline UnixSocket &GetSocket() { return clisock_; };

    inline void *GetRegion() { return alloc_->GetRegion(); }

    void RegisterQueuePair(labstor::ipc::queue_pair *qp) {
        qps_.resize(LABSTOR_MAX_QP_FLAG_COMBOS);
        qps_[qp->GetQid().flags_].resize(qp->GetQid().cnt_ + 1);
        qps_[qp->GetQid().flags_][qp->GetQid().cnt_] = qp;
    }
    labstor::ipc::queue_pair* GetQueuePair(labstor::ipc::qid_t qid) {
        return qps_[qid.flags_][qid.cnt_];
    }
};
}

#endif //LABSTOR_PER_PROCESS_IPC_H
