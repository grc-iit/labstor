
#ifndef LABSTOR_QUEUE_PAIR_KERNEL_H
#define LABSTOR_QUEUE_PAIR_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#include <labstor/types/data_structures/shmem_request_queue.h>
#include <labstor/types/data_structures/shmem_request_map.h>

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

struct labstor_queue_pair_ptr {
    labstor_off_t sq_off;
    labstor_off_t cq_off;
    uint32_t pid;
};

struct labstor_queue_pair {
    struct labstor_request_queue sq;
    struct labstor_unordered_map_request cq;
};

static inline void labstor_queue_pair_ptr_Init(struct labstor_queue_pair_ptr *ptr, labstor_qid_t qid, void *sq_region, void *cq_region, void *region) {
    ptr->sq_off = LABSTOR_REGION_SUB(sq_region, region);
    ptr->cq_off = LABSTOR_REGION_SUB(cq_region, region);
    ptr->pid = LABSTOR_GET_QP_PID(qid);
}

static inline void labstor_queue_pair_InitFromPtr(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *region) {
    labstor_request_queue_Attach(&qp->sq, LABSTOR_REGION_ADD(ptr->sq_off, region));
    labstor_unordered_map_request_Attach(&qp->cq, LABSTOR_REGION_ADD(ptr->cq_off, region));
}

static inline void labstor_queue_pair_GetPointer(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *region) {
    labstor_queue_pair_ptr_Init(
            ptr,
            labstor_request_queue_GetQid(&qp->sq),
            labstor_request_queue_GetRegion(&qp->sq),
            labstor_unordered_map_request_GetRegion(&qp->cq),
            region);
}

static inline void labstor_queue_pair_Init(struct labstor_queue_pair *qp, labstor_qid_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_request_queue_Init(&qp->sq, sq_region, sq_size, qid);
    labstor_unordered_map_request_Init(&qp->cq, cq_region, cq_size, 4);
}

static inline void labstor_queue_pair_Attach(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *base) {
    labstor_request_queue_Attach(&qp->sq, LABSTOR_REGION_ADD(ptr->sq_off, base));
    labstor_unordered_map_request_Attach(&qp->cq, LABSTOR_REGION_ADD(ptr->cq_off, base));
}

static inline struct labstor_qtok_t labstor_queue_pair_Enqueue(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    return labstor_request_queue_Enqueue(&qp->sq, rq);
}

static inline bool labstor_queue_pair_Dequeue(struct labstor_queue_pair *qp, struct labstor_request** rq) {
    return labstor_request_queue_Dequeue(&qp->sq, rq);
}

static inline void labstor_queue_pair_Complete(struct labstor_queue_pair *qp, struct labstor_request *rq, struct labstor_request *msg) {
    struct labstor_request_map_bucket b;
    msg->req_id_ = rq->req_id_;
    labstor_request_map_bucket_Init(&b, rq, labstor_unordered_map_request_GetRegion(&qp->cq));
    labstor_unordered_map_request_Set(&qp->cq, &b);
}

static inline struct labstor_request* labstor_queue_pair_Wait(struct labstor_queue_pair *qp, uint32_t req_id) {
    struct labstor_request *ret = NULL;
    while(!labstor_unordered_map_request_Find(&qp->cq, req_id, &ret)) {}
    labstor_unordered_map_request_Remove(&qp->cq, req_id);
    return ret;
}

static inline uint32_t labstor_queue_pair_GetDepth(struct labstor_queue_pair *qp) {
    return labstor_request_queue_GetDepth(&qp->sq);
}



#ifdef __cplusplus

namespace labstor::ipc {

struct queue_pair_ptr : public labstor_queue_pair_ptr {
    queue_pair_ptr() = default;
    queue_pair_ptr(qid_t qid, void *sq_region, void *cq_region, void *region) {
        Init(qid, sq_region, cq_region, region);
    }
    void Init(qid_t qid, void *sq_region, void *cq_region, void *region) {
        labstor_queue_pair_ptr_Init(this, qid, sq_region, cq_region, region);
    }
    uint32_t GetPID() {
        return pid;
    }
};

struct queue_pair : public labstor_queue_pair {
    inline queue_pair() = default;
    inline queue_pair(labstor::ipc::qid_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
        Init(qid, sq_region, sq_size, cq_region, cq_size);
    }
    inline queue_pair(queue_pair_ptr &ptr, void *region) {
        labstor_queue_pair_Init(this, &ptr, region);
    }

    inline void GetPointer(queue_pair_ptr &ptr, void *region) {
        labstor_queue_pair_GetPointer(this, &ptr, region);
    }

    inline void Init(labstor::ipc::qid_t qid, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
        labstor_queue_pair_GetPointer(this, &ptr, region);
    }

    inline void Attach(queue_pair_ptr &ptr, void *base) {
        labstor_queue_pair_Attach(this, &ptr, base);
    }

    inline qid_t GetQid() {
        return sq.GetQid();
    }

    inline labstor::ipc::qtok_t Enqueue(labstor::ipc::request *rq) {
        return sq.Enqueue(rq);
    }

    inline void Complete(labstor::ipc::request *rq, labstor::ipc::request *msg) {
        labstor_queue_pair_Complete(this, rq, msg);
    }

    inline labstor::ipc::request* Wait(uint32_t req_id) {
        return labstor_queue_pair_Wait(req_id);
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

#endif

#endif