//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_LABSTOR_QUEUE_PAIR_H
#define LABSTOR_LABSTOR_QUEUE_PAIR_H

#include <labstor/types/data_structures/queue_pair.h>
#include "labstor_request_queue.h"
#include "labstor_reqeust_map.h"

struct labstor_queue_pair {
    labstor_request_queue sq;
    labstor_request_map cq;
};

inline queue_pair(labstor::ipc::qid_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
Init(qid, sq_region, sq_size, cq_region, cq_size);
}
inline queue_pair(queue_pair_ptr &ptr, void *region) {
sq.Attach(LABSTOR_REGION_ADD(ptr.sq_off, region));
cq.Attach(LABSTOR_REGION_ADD(ptr.cq_off, region));
}

inline void GetPointer(queue_pair_ptr &ptr, void *region) {
    ptr.Init(sq.GetQid(), sq.GetRegion(), cq.GetRegion(), region);
}

inline void Init(labstor::ipc::qid_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    sq.Init(sq_region, sq_size, qid);
    cq.Init(cq_region, cq_size, 4);
}

inline void Attach(queue_pair_ptr &ptr, void *base) {
    sq.Attach(LABSTOR_REGION_ADD(ptr.sq_off, base));
    cq.Attach(LABSTOR_REGION_ADD(ptr.cq_off, base));
}

inline qid_t GetQid() {
    return sq.GetQid();
}

inline labstor::ipc::qtok_t Enqueue(labstor::ipc::request *rq) {
    return sq.Enqueue(rq);
}

inline void Complete(labstor::ipc::request *rq, labstor::ipc::request *msg) {
    msg->req_id_ = rq->req_id_;
    TRACEPOINT("labstor::ipc::queue_pair::Complete", GetQid(), msg->req_id_)
    cq.Set(msg);
}

inline labstor::ipc::request* Wait(uint32_t req_id) {
    labstor::ipc::request *ret = nullptr;
    while(!cq.Find(req_id, ret)) {}
    TRACEPOINT("labstor::ipc::queue_pair::Wait", GetQid(), req_id, (size_t)ret)
    cq.Remove(req_id);
    return ret;
}

static inline uint64_t GetStreamQueuePairID(labstor::ipc::qid_t flags, uint32_t hash, uint32_t num_qps, int pid) {
    uint32_t idx = hash % num_qps;
    return flags | ((idx << LABSTOR_QP_PID_BITS) + pid);
}
static inline uint64_t GetStreamQueuePairID(labstor::ipc::qid_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
    uint32_t hash = 0;
    for(int i = 0; i < str.size(); ++i) {
        hash += str[i] << 4*(i%4);
    }
    hash *= ns_id;
    return GetStreamQueuePairID(flags, hash, num_qps, pid);
}
static inline uint64_t GetStreamQueuePairOff(labstor::ipc::qid_t flags, uint32_t hash, uint32_t num_qps, int pid) {
    return GetStreamQueuePairID(flags, hash, num_qps, pid) % num_qps;
}
static inline uint64_t GetStreamQueuePairOff(labstor::ipc::qid_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
    return GetStreamQueuePairID(flags, str, ns_id, num_qps, pid) % num_qps;
}

#endif //LABSTOR_LABSTOR_QUEUE_PAIR_H
