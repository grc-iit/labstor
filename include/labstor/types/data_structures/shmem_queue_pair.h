//
// Created by lukemartinlogan on 11/20/21.
//

#ifndef LABSTOR_SHMEM_QUEUE_PAIR_H
#define LABSTOR_SHMEM_QUEUE_PAIR_H

#include "shmem_request_queue.h"
#include "shmem_request_map.h"
#include <string>

//qid: [flags (5-bit)][cnt (12-bit)][pid (15-bit)]

#define LABSTOR_QP_INTERMEDIATE 0x08000000
#define LABSTOR_QP_UNORDERED 0x10000000
#define LABSTOR_QP_BATCH 0x20000000
#define LABSTOR_QP_HIGH_LATENCY 0x40000000
#define LABSTOR_QP_PRIVATE 0x80000000

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

#define LABSTOR_GET_QP_IDX(flags) ((flags >> 15) & 0xFFF)

namespace labstor::ipc {

struct queue_pair_ptr {
    uint32_t sq_off;
    uint32_t cq_off;

    queue_pair_ptr() = default;
    queue_pair_ptr(void *sq_region, void *cq_region, void *region) {
        Init(sq_region, cq_region, region);
    }

    void Init(void *sq_region, void *cq_region, void *region) {
        sq_off = LABSTOR_REGION_SUB(sq_region, region);
        cq_off = LABSTOR_REGION_SUB(cq_region, region);
    }
};

struct queue_pair {
    request_queue sq;
    request_map cq;

    queue_pair() = default;
    queue_pair(uint32_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
        Init(qid, sq_region, sq_size, cq_region, cq_size);
    }

    void Init(uint32_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
        sq.Init(sq_region, sq_size, qid);
        cq.Init(cq_region, cq_size, 4);
    }

    void Attach(queue_pair_ptr &ptr, void *region) {
        sq.Attach(LABSTOR_REGION_ADD(ptr.sq_off, region));
        cq.Attach(LABSTOR_REGION_ADD(ptr.cq_off, region));
    }

    labstor::ipc::request* Wait(uint32_t qtok) {
        labstor::ipc::request *ret;
        while(!cq.Find(qtok, ret)) {}
        cq.Remove(qtok);
    }

    static inline uint64_t GetStreamQueuePairID(uint32_t flags, uint32_t hash, uint32_t num_qps, int pid) {
        uint32_t idx = hash % num_qps;
        return flags | ((idx << 15) + pid);
    }
    static inline uint64_t GetStreamQueuePairID(uint32_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
        uint32_t hash = 0;
        for(int i = 0; i < str.size(); ++i) {
            hash += str[i] << 4*(i%4);
        }
        hash *= ns_id;
        return GetStreamQueuePairID(flags, hash, num_qps, pid);
    }
    static inline uint64_t GetStreamQueuePairOff(uint32_t flags, uint32_t hash, uint32_t num_qps, int pid) {
        return GetStreamQueuePairID(flags, hash, num_qps, pid) % num_qps;
    }
    static inline uint64_t GetStreamQueuePairOff(uint32_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
        return GetStreamQueuePairID(flags, str, ns_id, num_qps, pid) % num_qps;
    }
};

}

#endif //LABSTOR_SHMEM_QUEUE_PAIR_H
