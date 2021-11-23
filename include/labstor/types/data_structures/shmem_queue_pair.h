//
// Created by lukemartinlogan on 11/20/21.
//

#ifndef LABSTOR_SHMEM_QUEUE_PAIR_H
#define LABSTOR_SHMEM_QUEUE_PAIR_H

#include "shmem_request_queue.h"
#include "shmem_request_map.h"
#include <string>

namespace labstor::ipc {

struct queue_pair {
    request_queue sq;
    request_map cq;

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

struct queue_pair_ptr {
    uint32_t sq_off;
    uint32_t cq_off;
};

}

#endif //LABSTOR_SHMEM_QUEUE_PAIR_H
