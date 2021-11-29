//
// Created by lukemartinlogan on 11/20/21.
//

#ifndef LABSTOR_SHMEM_QUEUE_PAIR_H
#define LABSTOR_SHMEM_QUEUE_PAIR_H

#include "shmem_request_queue.h"
#include "shmem_request_map.h"
#include <string>
#include <labstor/util/debug.h>

//qid: [flags (10-bit)][cnt (32-bit)][pid (22-bit)]

#define LABSTOR_QP_INTERMEDIATE (1ul << 63)
#define LABSTOR_QP_UNORDERED (1ul << 62)
#define LABSTOR_QP_BATCH (1ul << 61)
#define LABSTOR_QP_HIGH_LATENCY (1ul << 60)
#define LABSTOR_QP_PRIVATE (1ul << 59)

#define LABSTOR_QP_PRIMARY 0
#define LABSTOR_QP_ORDERED 0
#define LABSTOR_QP_STREAM 0
#define LABSTOR_QP_LOW_LATENCY 0
#define LABSTOR_QP_SHMEM 0

#define LABSTOR_QP_IS_INTERMEDIATE(flags) (flags & LABSTOR_QP_INTERMEDIATE)
#define LABSTOR_QP_IS_UNORDERED(flags) (flags & LABSTOR_QP_UNORDERED)
#define LABSTOR_QP_IS_BATCH(flags) (flags & LABSTOR_QP_BATCH)
#define LABSTOR_QP_IS_HIGH_LATENCY(flags) (flags & LABSTOR_QP_HIGH_LATENCY)
#define LABSTOR_QP_IS_PRIVATE(flags) (flags & LABSTOR_QP_PRIVATE)

#define LABSTOR_QP_IS_PRIMARY(flags) (!(flags & LABSTOR_QP_INTERMEDIATE))
#define LABSTOR_QP_IS_ORDERED(flags) (!(flags & LABSTOR_QP_UNORDERED))
#define LABSTOR_QP_IS_STREAM(flags) (!(flags & LABSTOR_QP_BATCH))
#define LABSTOR_QP_IS_LOW_LATENCY(flags) (!(flags & LABSTOR_QP_HIGH_LATENCY))
#define LABSTOR_QP_IS_SHMEM(flags) (!(flags & LABSTOR_QP_PRIVATE))

#define LABSTOR_QP_PID_BITS 22
#define LABSTOR_QP_COUNT_BITS 32
#define LABSTOR_QP_FLAG_BITS 10

#define LABSTOR_GET_QP_IDX(qid) ((qid >> LABSTOR_QP_PID_BITS) & 0xFFFFFFFF)
#define LABSTOR_GET_QP_PID(qid) (qid & 0x3FFFFF)

namespace labstor::ipc {

struct queue_pair_ptr {
    labstor::off_t sq_off;
    labstor::off_t cq_off;
    uint32_t pid;

    queue_pair_ptr() = default;
    queue_pair_ptr(qid_t qid, void *sq_region, void *cq_region, void *region) {
        Init(qid, sq_region, cq_region, region);
    }
    void Init(qid_t qid, void *sq_region, void *cq_region, void *region) {
        sq_off = LABSTOR_REGION_SUB(sq_region, region);
        cq_off = LABSTOR_REGION_SUB(cq_region, region);
        pid = LABSTOR_GET_QP_PID(qid);
    }
    uint32_t GetPID() {
        return pid;
    }
};

struct queue_pair {
    request_queue sq;
    request_map cq;

    queue_pair() = default;
    queue_pair(labstor::ipc::qid_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
        Init(qid, sq_region, sq_size, cq_region, cq_size);
    }
    queue_pair(queue_pair_ptr &ptr, void *region) {
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
        TRACEPOINT("")
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
        msg->qtok_ = rq->qtok_;
        cq.Set(msg);
    }

    inline labstor::ipc::request* Wait(uint32_t qtok) {
        labstor::ipc::request *ret;
        while(!cq.Find(qtok, ret)) {}
        cq.Remove(qtok);
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
};

}

#endif //LABSTOR_SHMEM_QUEUE_PAIR_H
