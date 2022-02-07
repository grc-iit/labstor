
#ifndef LABSTOR_QUEUE_PAIR_H
#define LABSTOR_QUEUE_PAIR_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#include <labstor/types/data_structures/spsc/shmem_request_queue.h>
#include <labstor/types/data_structures/spsc/shmem_request_map.h>
#include <labstor/constants/debug.h>
#include <labstor/userspace/util/errors.h>


/*QUEUE FLAGS & ID*/

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

#define LABSTOR_GET_QP_IPC_ID_BITS 22
#define LABSTOR_QP_COUNT_BITS 32
#define LABSTOR_QP_FLAG_BITS 10

#define LABSTOR_GET_QP_IDX(qid) ((qid >> LABSTOR_GET_QP_IPC_ID_BITS) & 0xFFFFFFFF)
#define LABSTOR_GET_QP_IPC_ID(qid) (qid & 0x3FFFFF)

/*QUEUE DEFINITION*/

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
    struct labstor_request_map cq;
#ifdef __cplusplus
    inline labstor_queue_pair() = default;
    inline labstor_queue_pair(labstor::ipc::qid_t qid, void *base_region, uint32_t depth, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
        Init(qid, base_region, depth, sq_region, sq_size, cq_region, cq_size);
    }
    inline labstor_queue_pair(labstor::ipc::qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
        Init(qid, base_region, sq_region, sq_size, cq_region, cq_size);
    }
    inline labstor_queue_pair(labstor::ipc::queue_pair_ptr &ptr, void *base_region) {
        Attach(ptr, base_region);
    }
    static inline uint32_t GetSize(uint32_t queue_depth);
    inline void GetPointer(labstor::ipc::queue_pair_ptr &ptr, void *region);
    inline uint32_t GetDepth();
    inline int GetPID();
    inline void Init(labstor::ipc::qid_t qid, void *base_region, uint32_t depth, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size);
    inline void Init(labstor::ipc::qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size);
    inline void Attach(labstor::ipc::queue_pair_ptr &ptr, void *base_region);
    inline void RemoteAttach(labstor::ipc::queue_pair_ptr &ptr, void *kern_base_region);
    inline labstor::ipc::qid_t GetQid();
    template<typename T>
    inline void Enqueue(T *rq, labstor::ipc::qtok_t &qtok);
    template<typename T>
    inline bool Dequeue(T *&rq);
    template<typename S, typename T=S>
    inline void Complete(S *old_rq, T *new_rq);
    template<typename T>
    inline void Complete(T *rq);
    template<typename T>
    inline void Complete(labstor::ipc::qtok_t &qtok, T *rq);
    template<typename T>
    inline bool IsComplete(labstor::ipc::qtok_t &qtok, T *&rq);
    template<typename T>
    inline T* Wait(uint32_t req_id);
    template<typename T>
    inline T* Wait(uint32_t req_id, uint32_t max_ms);
    template<typename T>
    inline T* Wait(labstor::ipc::qtok_t &qtok);
    template<typename T>
    inline T* Wait(labstor::ipc::qtok_t &qtok, uint32_t max_ms);
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
    ptr->pid = LABSTOR_GET_QP_IPC_ID(qid);
}

static inline uint32_t labstor_queue_pair_GetSize_global(uint32_t queue_depth) {
    return sizeof(struct labstor_queue_pair) +
        labstor_request_queue_GetSize_global(queue_depth) +
        labstor_request_map_GetSize_global(queue_depth);
}

static inline void labstor_queue_pair_GetPointer(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *base_region) {
    labstor_queue_pair_ptr_Init(
            ptr,
            labstor_request_queue_GetQid(&qp->sq),
            labstor_request_queue_GetRegion(&qp->sq),
            labstor_request_map_GetRegion(&qp->cq),
            base_region);
}

static inline void labstor_queue_pair_Init(
        struct labstor_queue_pair *qp, labstor_qid_t qid, void *base_region, uint32_t depth, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_request_queue_Init(&qp->sq, base_region, sq_region, sq_size, depth, qid);
    labstor_request_map_Init(&qp->cq, base_region, cq_region, cq_size, depth);
}

static inline void labstor_queue_pair_Attach(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *base_region) {
    labstor_request_queue_Attach(&qp->sq, base_region, LABSTOR_REGION_ADD(ptr->sq_off, base_region));
    labstor_request_map_Attach(&qp->cq, base_region, LABSTOR_REGION_ADD(ptr->cq_off, base_region));
}

static inline void labstor_queue_pair_RemoteAttach(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *kern_base_region) {
    labstor_request_queue_RemoteAttach(&qp->sq, kern_base_region, LABSTOR_REGION_ADD(ptr->sq_off, kern_base_region));
    labstor_request_map_RemoteAttach(&qp->cq, kern_base_region, LABSTOR_REGION_ADD(ptr->cq_off, kern_base_region));
}

static inline bool labstor_queue_pair_Enqueue(struct labstor_queue_pair *qp, struct labstor_request *rq, struct labstor_qtok_t *qtok) {
    //AUTO_TRACE("")
    return labstor_request_queue_Enqueue(&qp->sq, rq, qtok);
}

static inline bool labstor_queue_pair_Dequeue(struct labstor_queue_pair *qp, struct labstor_request** rq) {
    //AUTO_TRACE("")
    return labstor_request_queue_Dequeue(&qp->sq, rq);
}

static inline bool labstor_queue_pair_CompleteTimed(struct labstor_queue_pair *qp, int req_id, struct labstor_request *rq) {
    LABSTOR_TIMED_SPINWAIT_PREAMBLE()
    rq->req_id_ = req_id;
    LABSTOR_TIMED_SPINWAIT_START(50)
    if(labstor_request_map_Set(&qp->cq, rq)) {
        return true;
    }
    LABSTOR_TIMED_SPINWAIT_END(50)
    return false;
}

static inline bool labstor_queue_pair_CompleteInf(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    LABSTOR_INF_SPINWAIT_PREAMBLE()
    LABSTOR_INF_SPINWAIT_START()
    if(labstor_request_map_Set(&qp->cq, rq)) {
        return true;
    }
    LABSTOR_INF_SPINWAIT_END()
    return false;
}

static inline bool labstor_queue_pair_CompleteQuick(struct labstor_queue_pair *qp, struct labstor_request *old_rq, struct labstor_request *new_rq) {
    new_rq->req_id_ = old_rq->req_id_;
    return labstor_queue_pair_CompleteInf(qp, new_rq);
}

static inline bool labstor_queue_pair_CompleteByQtok(struct labstor_queue_pair *qp, struct labstor_qtok_t *qtok, struct labstor_request *rq) {
    rq->req_id_ = qtok->req_id_;
    return labstor_queue_pair_CompleteInf(qp, rq);
}

static inline bool labstor_queue_pair_Complete(struct labstor_queue_pair *qp,  struct labstor_request *old_rq, struct labstor_request *new_rq) {
    new_rq->req_id_ = old_rq->req_id_;
    return labstor_queue_pair_CompleteInf(qp, new_rq);
}

static inline bool labstor_queue_pair_IsComplete(struct labstor_queue_pair *qp, uint32_t req_id, struct labstor_request **rq) {
    return labstor_request_map_FindAndRemove(&qp->cq, req_id, rq);
}

static inline struct labstor_request* labstor_queue_pair_Wait(struct labstor_queue_pair *qp, uint32_t req_id) {
    LABSTOR_INF_SPINWAIT_PREAMBLE()
    struct labstor_request *ret = NULL;
    LABSTOR_INF_SPINWAIT_START()
    if(labstor_queue_pair_IsComplete(qp, req_id, &ret)) {
        return ret;
    }
    LABSTOR_INF_SPINWAIT_END()
    return NULL;
}
static inline struct labstor_request* labstor_queue_pair_TimedWait(struct labstor_queue_pair *qp, uint32_t req_id, uint32_t max_ms) {
    LABSTOR_TIMED_SPINWAIT_PREAMBLE()
    struct labstor_request *ret = NULL;
    LABSTOR_TIMED_SPINWAIT_START(max_ms)
    if(labstor_queue_pair_IsComplete(qp, req_id, &ret)) {
        return ret;
    }
    LABSTOR_TIMED_SPINWAIT_END(max_ms)
    return NULL;
}

static inline uint32_t labstor_queue_pair_GetDepth(struct labstor_queue_pair *qp) {
    return labstor_request_queue_GetDepth(&qp->sq);
}

static inline labstor_qid_t labstor_queue_pair_GetQid(struct labstor_queue_pair *qp) {
    return labstor_request_queue_GetQid(&qp->sq);
}

static inline int labstor_queue_pair_GetPID(struct labstor_queue_pair *qp) {
    return LABSTOR_GET_QP_IPC_ID(labstor_queue_pair_GetQid(qp));
}



#ifdef __cplusplus

/*queue_pair_ptr*/

void labstor::ipc::queue_pair_ptr::Init(labstor::ipc::qid_t qid, void *sq_region, void *cq_region, void *base_region) {
    labstor_queue_pair_ptr_Init(this, qid, sq_region, cq_region, base_region);
}

/*queue_pair*/

uint32_t labstor::ipc::queue_pair::GetSize(uint32_t queue_depth) {
    return labstor_queue_pair_GetSize_global(queue_depth);
}

void labstor::ipc::queue_pair::GetPointer(labstor::ipc::queue_pair_ptr &ptr, void *base_region) {
    labstor_queue_pair_GetPointer(this, &ptr, base_region);
}

int labstor::ipc::queue_pair::GetPID() {
    return labstor_queue_pair_GetPID(this);
}

uint32_t labstor::ipc::queue_pair::GetDepth() {
    return labstor_queue_pair_GetDepth(this);
}

void labstor::ipc::queue_pair::Init(labstor::ipc::qid_t qid, void *base_region, uint32_t depth, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_queue_pair_Init(this, qid, base_region, depth, sq_region, sq_size, cq_region, cq_size);
}

void labstor::ipc::queue_pair::Init(labstor::ipc::qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_queue_pair_Init(this, qid, base_region, 0, sq_region, sq_size, cq_region, cq_size);
}

void labstor::ipc::queue_pair::Attach(labstor::ipc::queue_pair_ptr &ptr, void *base_region) {
    labstor_queue_pair_Attach(this, &ptr, base_region);
}

void labstor::ipc::queue_pair::RemoteAttach(labstor::ipc::queue_pair_ptr &ptr, void *kern_base_region) {
    labstor_queue_pair_RemoteAttach(this, &ptr, kern_base_region);
}

labstor::ipc::qid_t labstor::ipc::queue_pair::GetQid() {
    return labstor_queue_pair_GetQid(this);
}

template<typename T>
void labstor::ipc::queue_pair::Enqueue(T *rq, labstor::ipc::qtok_t &qtok) {
    if(!labstor_queue_pair_Enqueue(this, reinterpret_cast<labstor::ipc::request*>(rq), &qtok)) {
        throw labstor::FAILED_TO_ENQUEUE.format();
    }
}

template<typename T>
bool labstor::ipc::queue_pair::Dequeue(T *&rq) {
    return labstor_queue_pair_Dequeue(this, reinterpret_cast<labstor::ipc::request**>(&rq));
}

template<typename S, typename T=S>
void labstor::ipc::queue_pair::Complete(S *old_rq, T *new_rq) {
    if(!labstor_queue_pair_Complete(this, reinterpret_cast<labstor::ipc::request*>(old_rq), reinterpret_cast<labstor::ipc::request*>(new_rq))) {
        throw labstor::FAILED_TO_COMPLETE.format();
    }
}

template<typename T>
void labstor::ipc::queue_pair::Complete(T *rq) {
    if(!labstor_queue_pair_CompleteInf(this, reinterpret_cast<labstor::ipc::request*>(rq))) {
        throw labstor::FAILED_TO_COMPLETE.format();
    }
}

template<typename T>
void labstor::ipc::queue_pair::Complete(labstor::ipc::qtok_t &qtok, T *rq) {
    if(!labstor_queue_pair_CompleteByQtok(this, &qtok, reinterpret_cast<labstor::ipc::request*>(rq))) {
        throw labstor::FAILED_TO_COMPLETE.format();
    }
}

template<typename T>
bool labstor::ipc::queue_pair::IsComplete(labstor::ipc::qtok_t &qtok, T *&rq) {
    return labstor_queue_pair_IsComplete(this, qtok.req_id_, reinterpret_cast<labstor::ipc::request**>(&rq));
}

template<typename T>
T* labstor::ipc::queue_pair::Wait(uint32_t req_id) {
    T* ptr = reinterpret_cast<T*>(labstor_queue_pair_Wait(this, req_id));
    if(ptr == NULL) {
        throw labstor::QP_WAIT_TIMED_OUT;
    }
    return ptr;
}

template<typename T>
T* labstor::ipc::queue_pair::Wait(uint32_t req_id, uint32_t max_ms) {
    T* ptr = reinterpret_cast<T*>(labstor_queue_pair_TimedWait(this, req_id, max_ms));
    if(ptr == NULL) {
        throw labstor::QP_WAIT_TIMED_OUT;
    }
    return ptr;
}

template<typename T>
T* labstor::ipc::queue_pair::Wait(labstor::ipc::qtok_t &qtok) {
    return Wait<T>(qtok.req_id_);
}

template<typename T>
inline T* labstor::ipc::queue_pair::Wait(labstor::ipc::qtok_t &qtok, uint32_t max_ms) {
    return Wait<T>(qtok.req_id_, max_ms);
}

uint64_t labstor::ipc::queue_pair::GetStreamQueuePairID(labstor::ipc::qid_t flags, uint32_t hash, uint32_t num_qps, int pid) {
    uint32_t idx = hash % num_qps;
    return flags | ((idx << LABSTOR_GET_QP_IPC_ID_BITS) + pid);
}
uint64_t labstor::ipc::queue_pair::GetStreamQueuePairID(labstor::ipc::qid_t flags, const std::string &str, uint32_t ns_id, uint32_t num_qps, int pid) {
    uint32_t hash = 0;
    for(size_t i = 0; i < str.size(); ++i) {
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