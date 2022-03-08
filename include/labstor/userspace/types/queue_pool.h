//
// Created by lukemartinlogan on 3/3/22.
//

#ifndef LABSTOR_QUEUE_POOL_H
#define LABSTOR_QUEUE_POOL_H

#include <labstor/types/data_structures/queue_pair.h>
#include <labstor/types/thread_local.h>
#include <vector>

namespace labstor {

class QueuePool {
private:
    std::unordered_map<std::string, int> qp_type_to_id_;
    std::vector<std::vector<std::vector<labstor::queue_pair*>>> qps_;
public:
    QueuePool() {
        RegisterQueuePairType("LabStor", LABSTOR_QP_ALL_FLAGS);
    }

    inline int RegisterQueuePairType(const std::string &&qp_type, labstor_qid_flags_t possible_flags) {
        qp_type_to_id_.emplace(qp_type, qp_type_to_id_.size());
        qps_.emplace_back();
        qps_.back().resize(possible_flags + 1);
        return qp_type_to_id_[qp_type];
    }
    inline void ReserveQueues(int type, labstor_qid_flags_t flags, int n) {
        TRACEPOINT("Type", type, "Flags", flags, "n", n, "num_types", qps_.size())
        if(type >= qps_.size()) {
            throw INVALID_QP_QUERY.format();
        }
        auto &qps_type = qps_[type];
        TRACEPOINT("Num Possible Flags for Type", qps_type.size())
        if(flags >= qps_type.size()) {
            throw INVALID_QP_QUERY.format();
        }
        auto &qps_flags = qps_type[flags];
        TRACEPOINT("Num Queues of (type,flags)", qps_flags.size())
        qps_flags.reserve(n);
    }
    inline int GetQueuePairTypeID(const std::string &&qp_type) {
        return qp_type_to_id_[qp_type];
    }
    inline void RegisterQueuePair(labstor::queue_pair *qp) {
        auto &qid = qp->GetQID();;
        if(qid.type_ >= qps_.size()) {
            throw INVALID_QP_QUERY.format(qid.pid_, qid.type_, qid.flags_, qid.cnt_);
        }
        auto &qps_type = qps_[qid.type_];
        if(qid.flags_ >= qps_type.size()) {
            throw INVALID_QP_QUERY.format(qid.pid_, qid.type_, qid.flags_, qid.cnt_);
        }
        auto &qps_flags = qps_type[qid.flags_];
        qps_flags.emplace_back(qp);
    }
    inline void GetQueuePair(labstor::queue_pair *&qp, labstor_qid_type_t type, labstor_qid_flags_t flags, int off) {
        if(type >= qps_.size()) {
            throw INVALID_QP_QUERY.format(-1000, type, flags, off);
        }
        auto &qps_type = qps_[type];
        if(flags >= qps_type.size()) {
            throw INVALID_QP_QUERY.format(-1000, type, flags, off);
        }
        auto &qps_flags = qps_type[flags];
        if(off >= qps_flags.size()) {
            throw INVALID_QP_QUERY.format(-1000, type, flags, off);
        }
        qp = qps_flags[off];
    }
    inline void GetQueuePair(labstor::queue_pair *&qp, labstor::ipc::qid_t &qid) {
        GetQueuePair(qp, qid.type_, qid.flags_, qid.cnt_);
    }
    inline void GetQueuePair(labstor::queue_pair *&qp, labstor::ipc::qtok_t &qtok) {
        GetQueuePair(qp, qtok.qid_);
    }
    inline int GetNumQueuePairs(labstor_qid_type_t type, labstor_qid_flags_t flags) {
        if(type >= qps_.size()) {
            throw INVALID_QP_QUERY.format();
        }
        auto &qps_type = qps_[type];
        if(flags >= qps_type.size()) {
            throw INVALID_QP_QUERY.format();
        }
        auto &qps_flags = qps_type[flags];
        return (int)qps_flags.size();
    }
    inline int GetNumQueuePairsFast(labstor_qid_type_t type, labstor_qid_flags_t flags) {
        return (int)qps_[type][flags].size();
    }
};

}

#endif //LABSTOR_QUEUE_POOL_H
