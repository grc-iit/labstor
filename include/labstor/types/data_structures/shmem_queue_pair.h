
#ifndef LABSTOR_QUEUE_PAIR_KERNEL_H
#define LABSTOR_QUEUE_PAIR_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#include <labstor/types/data_structures/shmem_request_queue.h>
#include <labstor/types/data_structures/shmem_request_map.h>
#include <labstor/userspace/util/debug.h>

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


//Define the labstor::ipc::queue_pair_ptr type
struct labstor_queue_pair_ptr {
    labstor_off_t sq_off;
    labstor_off_t cq_off;
    uint32_t pid;
#ifdef __cplusplus
    labstor_queue_pair_ptr() = default;
    inline labstor_queue_pair_ptr(labstor::ipc::qid_t qid, void *sq_region, void *cq_region, void *region) {
        Init(qid, sq_region, cq_region, region);
    }
    inline uint32_t GetPID() {
        return pid;
    }
    inline void Init(labstor::ipc::qid_t qid, void *sq_region, void *cq_region, void *region);
#endif
};

#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_queue_pair_ptr queue_pair_ptr;
}
#endif

//Define the labstor::ipc::queue_pair type
struct labstor_queue_pair {
    struct labstor_request_queue sq;
    struct labstor_unordered_map_uint32_t_request cq;
#ifdef __cplusplus
    inline labstor_queue_pair() = default;
    inline labstor_queue_pair(labstor::ipc::qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
        Init(qid, base_region, sq_region, sq_size, cq_region, cq_size);
    }
    inline labstor_queue_pair(labstor::ipc::queue_pair_ptr &ptr, void *base_region) {
        Attach(ptr, base_region);
    }
    inline void GetPointer(labstor::ipc::queue_pair_ptr &ptr, void *region);
    inline uint32_t GetDepth();
    inline void Init(labstor::ipc::qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size);
    inline void Attach(labstor::ipc::queue_pair_ptr &ptr, void *base_region);
    inline labstor::ipc::qid_t GetQid();
    inline labstor::ipc::qtok_t Enqueue(labstor::ipc::request *rq);
    inline bool Dequeue(labstor::ipc::request *&rq);
    inline void Complete(labstor::ipc::request *rq, labstor::ipc::request *msg);
    inline labstor::ipc::request* Wait(uint32_t req_id);
    static inline uint64_t GetStreamQueuePairID(labstor::ipc::qid_t flags, uint32_t hash, uint32_t num_qps, int pid);
    static inline uint64_t GetStreamQueuePairID(labstor::ipc::qid_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid);
    static inline uint64_t GetStreamQueuePairOff(labstor::ipc::qid_t flags, uint32_t hash, uint32_t num_qps, int pid);
    static inline uint64_t GetStreamQueuePairOff(labstor::ipc::qid_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid);
#endif
};

#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_queue_pair queue_pair;
}
#endif

static inline void labstor_queue_pair_ptr_Init(struct labstor_queue_pair_ptr *ptr, labstor_qid_t qid, void *sq_region, void *cq_region, void *base_region) {
    ptr->sq_off = LABSTOR_REGION_SUB(sq_region, base_region);
    ptr->cq_off = LABSTOR_REGION_SUB(cq_region, base_region);
    ptr->pid = LABSTOR_GET_QP_PID(qid);
}

static inline void labstor_queue_pair_GetPointer(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *base_region) {
    labstor_queue_pair_ptr_Init(
            ptr,
            labstor_request_queue_GetQid(&qp->sq),
            labstor_request_queue_GetRegion(&qp->sq),
            labstor_unordered_map_uint32_t_request_GetRegion(&qp->cq),
            base_region);
}

static inline void labstor_queue_pair_Init(struct labstor_queue_pair *qp, labstor_qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_request_queue_Init(&qp->sq, base_region, sq_region, sq_size, qid);
    labstor_unordered_map_uint32_t_request_Init(&qp->cq, base_region, cq_region, cq_size, 4);
}

static inline void labstor_queue_pair_Attach(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *base_region) {
    labstor_request_queue_Attach(&qp->sq, base_region, LABSTOR_REGION_ADD(ptr->sq_off, base_region));
    labstor_unordered_map_uint32_t_request_Attach(&qp->cq, base_region, LABSTOR_REGION_ADD(ptr->cq_off, base_region));
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
    labstor_request_map_bucket_Init(&b, msg, labstor_unordered_map_uint32_t_request_GetBaseRegion(&qp->cq));
    TRACEPOINT("queue_pair_Complete", b.off_);
    labstor_unordered_map_uint32_t_request_Set(&qp->cq, &b);
}

static inline struct labstor_request* labstor_queue_pair_Wait(struct labstor_queue_pair *qp, uint32_t req_id) {
    struct labstor_request *ret = NULL;
    while(!labstor_unordered_map_uint32_t_request_Find(&qp->cq, req_id, &ret)) {}
    labstor_unordered_map_uint32_t_request_Remove(&qp->cq, req_id);
    return ret;
}

static inline uint32_t labstor_queue_pair_GetDepth(struct labstor_queue_pair *qp) {
    return labstor_request_queue_GetDepth(&qp->sq);
}

static inline labstor_qid_t labstor_queue_pair_GetQid(struct labstor_queue_pair *qp) {
    return labstor_request_queue_GetQid(&qp->sq);
}



#ifdef __cplusplus

/*queue_pair_ptr*/

void labstor::ipc::queue_pair_ptr::Init(labstor::ipc::qid_t qid, void *sq_region, void *cq_region, void *base_region) {
    labstor_queue_pair_ptr_Init(this, qid, sq_region, cq_region, base_region);
}

/*queue_pair*/

void labstor::ipc::queue_pair::GetPointer(labstor::ipc::queue_pair_ptr &ptr, void *base_region) {
    labstor_queue_pair_GetPointer(this, &ptr, base_region);
}

uint32_t labstor::ipc::queue_pair::GetDepth() {
    return labstor_queue_pair_GetDepth(this);
}

void labstor::ipc::queue_pair::Init(labstor::ipc::qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_queue_pair_Init(this, qid, base_region, sq_region, sq_size, cq_region, cq_size);
}

void labstor::ipc::queue_pair::Attach(labstor::ipc::queue_pair_ptr &ptr, void *base_region) {
    labstor_queue_pair_Attach(this, &ptr, base_region);
}

labstor::ipc::qid_t labstor::ipc::queue_pair::GetQid() {
    return labstor_queue_pair_GetQid(this);
}

labstor::ipc::qtok_t labstor::ipc::queue_pair::Enqueue(labstor::ipc::request *rq) {
    return labstor_request_queue_Enqueue(&sq, rq);
}

bool labstor::ipc::queue_pair::Dequeue(labstor::ipc::request *&rq) {
    return labstor_request_queue_Dequeue(&sq, reinterpret_cast<labstor_request**>(&rq));
}

void labstor::ipc::queue_pair::Complete(labstor::ipc::request *rq, labstor::ipc::request *msg) {
    labstor_queue_pair_Complete(this, rq, msg);
}

labstor::ipc::request* labstor::ipc::queue_pair::Wait(uint32_t req_id) {
    return reinterpret_cast<labstor::ipc::request*>(labstor_queue_pair_Wait(this, req_id));
}

uint64_t labstor::ipc::queue_pair::GetStreamQueuePairID(labstor::ipc::qid_t flags, uint32_t hash, uint32_t num_qps, int pid) {
    uint32_t idx = hash % num_qps;
    return flags | ((idx << LABSTOR_QP_PID_BITS) + pid);
}
uint64_t labstor::ipc::queue_pair::GetStreamQueuePairID(labstor::ipc::qid_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
    uint32_t hash = 0;
    for(int i = 0; i < str.size(); ++i) {
        hash += str[i] << 4*(i%4);
    }
    hash *= ns_id;
    return GetStreamQueuePairID(flags, hash, num_qps, pid);
}
uint64_t labstor::ipc::queue_pair::GetStreamQueuePairOff(labstor::ipc::qid_t flags, uint32_t hash, uint32_t num_qps, int pid) {
    return GetStreamQueuePairID(flags, hash, num_qps, pid) % num_qps;
}
uint64_t labstor::ipc::queue_pair::GetStreamQueuePairOff(labstor::ipc::qid_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
    return GetStreamQueuePairID(flags, str, ns_id, num_qps, pid) % num_qps;
}

#endif

#endif