//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_PER_PROCESS_IPC_H
#define LABSTOR_PER_PROCESS_IPC_H

#include <labstor/types/allocator/allocator.h>
#include <labstor/types/allocator/segment_allocator.h>
#include <labstor/userspace/util/errors.h>
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

    PerProcessIPC(int pid) : num_stream_qps_(0), qp_alloc_(nullptr), alloc_(nullptr) {
        creds_.pid_ = pid;
    }

    PerProcessIPC(int fd, labstor::credentials creds) : clisock_(fd), creds_(creds), num_stream_qps_(0) {}

    PerProcessIPC(const PerProcessIPC &old_ipc) {
        clisock_ = old_ipc.clisock_;
        creds_ = old_ipc.creds_;
        alloc_ = old_ipc.alloc_;
        num_stream_qps_ = old_ipc.num_stream_qps_;
    }

    inline UnixSocket &GetSocket() { return clisock_; };

    inline void *GetRegion() { return alloc_->GetRegion(); }

    inline int GetPID() { return creds_.pid_; }

    void RegisterQueuePair(labstor::ipc::queue_pair *qp) {
        qps_.resize(LABSTOR_MAX_QP_FLAG_COMBOS);
        auto qid = qp->GetQid();
        if(qid.flags_ >= qps_.size()) {
            throw INVALID_QP_FLAGS.format(qid.flags_);
        }
        auto &qps_by_flags = qps_[qid.flags_];
        qps_by_flags.resize(qp->GetQid().cnt_ + 1);
        qps_[qp->GetQid().flags_][qp->GetQid().cnt_] = qp;
        ++num_stream_qps_;
    }

    labstor::ipc::queue_pair* GetQueuePair(labstor::ipc::qid_t qid) {
        if(qid.flags_ >= qps_.size()) {
            throw INVALID_QP_FLAGS.format(qid.flags_);
        }
        auto &qps_by_flags = qps_[qid.flags_];
        if(qid.cnt_ >= qps_by_flags.size()) {
            throw INVALID_QP_CNT.format(qid.cnt_);
        }
        return qps_by_flags[qid.cnt_];
    }
};
}

#endif //LABSTOR_PER_PROCESS_IPC_H
