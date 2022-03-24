
#ifndef LABSTOR_SHMEM_QUEUE_PAIR1_H
#define LABSTOR_SHMEM_QUEUE_PAIR1_H

#include "labstor/types/basics.h"
#include "labstor/types/data_structures/shmem_qtok.h"
#include "shmem_request_queue.h"
#include "shmem_request_map.h"
#include "labstor/constants/debug.h"
#include "labstor/userspace/util/errors.h"
#include "labstor/types/data_structures/queue_pair.h"


/*QUEUE FLAGS & ID*/

#define LABSTOR_MAX_QP_FLAG_COMBOS 32
#define LABSTOR_QP_INTERMEDIATE (1ul << 4)
#define LABSTOR_QP_UNORDERED (1ul << 3)
#define LABSTOR_QP_BATCH (1ul << 2)
#define LABSTOR_QP_HIGH_LATENCY (1ul << 1)
#define LABSTOR_QP_PRIVATE (1ul << 0)

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

#define LABSTOR_QP_COUNT_BITS 32
#define LABSTOR_QP_ALL_FLAGS 31

#define LABSTOR_GET_QP_IDX(qid) (qid.cnt_)
#define LABSTOR_GET_QP_IPC_ID(qid) ((qid).pid_)

/*QUEUE DEFINITION*/

//Define the labstor::ipc::queue_pair_ptr type
struct labstor_queue_pair_ptr {
    labstor_off_t sq_off_;
    labstor_off_t cq_off_;
    uint32_t pid_;
#ifdef __cplusplus
    labstor_queue_pair_ptr() = default;
    inline labstor_queue_pair_ptr(labstor::ipc::qid_t qid, void *sq_region, void *cq_region, void *region) {
        Init(qid, sq_region, cq_region, region);
    }
    inline uint32_t GetPID() {
        return pid_;
    }
    inline void Init(labstor::ipc::qid_t qid, void *sq_region, void *cq_region, void *region);
#endif
};

#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_queue_pair_ptr queue_pair_ptr;
}
#endif

//Define the labstor::queue_pair type
struct labstor_queue_pair {
    struct labstor_request_queue sq_;
    struct labstor_request_map cq_;

#ifdef __cplusplus
    inline labstor::ipc::qid_t& GetQID();
    inline static uint32_t GetSize(uint32_t queue_depth);
    inline void GetPointer(labstor::ipc::queue_pair_ptr &ptr, void *base_region);
    inline uint32_t GetDepth();
    inline void Init(labstor::ipc::qid_t qid, void *base_region, uint32_t depth, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size);
    inline void Init(labstor::ipc::qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size);
    inline void Attach(labstor::ipc::queue_pair_ptr &ptr, void *base_region);
    inline void RemoteAttach(labstor::ipc::queue_pair_ptr &ptr, void *kern_base_region);
#endif
};

static inline void labstor_queue_pair_ptr_Init(struct labstor_queue_pair_ptr *ptr, labstor_qid_t qid, void *sq_region, void *cq_region, void *base_region) {
    ptr->sq_off_ = LABSTOR_REGION_SUB(sq_region, base_region);
    ptr->cq_off_ = LABSTOR_REGION_SUB(cq_region, base_region);
    ptr->pid_ = LABSTOR_GET_QP_IPC_ID(qid);
}

static inline uint32_t labstor_queue_pair_GetSize_global(uint32_t queue_depth) {
    return sizeof(struct labstor_queue_pair) +
        labstor_request_queue_GetSize_global(queue_depth) +
        labstor_request_map_GetSize_global(queue_depth);
}

static inline void labstor_queue_pair_GetPointer(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *base_region) {
    labstor_queue_pair_ptr_Init(
            ptr,
            *labstor_request_queue_GetQID(&qp->sq_),
            labstor_request_queue_GetRegion(&qp->sq_),
            labstor_request_map_GetRegion(&qp->cq_),
            base_region);
}

static inline void labstor_queue_pair_Init(
        struct labstor_queue_pair *qp, labstor_qid_t qid, void *base_region, uint32_t depth, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_request_queue_Init(&qp->sq_, base_region, sq_region, sq_size, depth, qid);
    labstor_request_map_Init(&qp->cq_, base_region, cq_region, cq_size, depth);
}

static inline void labstor_queue_pair_Attach(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *base_region) {
    labstor_request_queue_Attach(&qp->sq_, base_region, LABSTOR_REGION_ADD(ptr->sq_off_, base_region));
    labstor_request_map_Attach(&qp->cq_, base_region, LABSTOR_REGION_ADD(ptr->cq_off_, base_region));
}

static inline void labstor_queue_pair_RemoteAttach(struct labstor_queue_pair *qp, struct labstor_queue_pair_ptr *ptr, void *kern_base_region) {
    labstor_request_queue_RemoteAttach(&qp->sq_, kern_base_region, LABSTOR_REGION_ADD(ptr->sq_off_, kern_base_region));
    labstor_request_map_RemoteAttach(&qp->cq_, kern_base_region, LABSTOR_REGION_ADD(ptr->cq_off_, kern_base_region));
}

static inline bool labstor_queue_pair_Enqueue(struct labstor_queue_pair *qp, struct labstor_request *rq, struct labstor_qtok_t *qtok) {
    //AUTO_TRACE("")
    return labstor_request_queue_Enqueue(&qp->sq_, rq, qtok);
}

static inline bool labstor_queue_pair_Peek(struct labstor_queue_pair *qp, struct labstor_request** rq, int i) {
    return labstor_request_queue_Peek(&qp->sq_, rq, i);
}

static inline bool labstor_queue_pair_Dequeue(struct labstor_queue_pair *qp, struct labstor_request** rq) {
    //AUTO_TRACE("")
    return labstor_request_queue_Dequeue(&qp->sq_, rq);
}

static inline bool labstor_queue_pair_CompleteTimed(struct labstor_queue_pair *qp, int req_id, struct labstor_request *rq) {
    LABSTOR_TIMED_SPINWAIT_PREAMBLE()
    rq->req_id_ = req_id;
    LABSTOR_TIMED_SPINWAIT_START(50)
    if(labstor_request_map_Set(&qp->cq_, rq)) {
        return true;
    }
    LABSTOR_TIMED_SPINWAIT_END(50)
    return false;
}

static inline bool labstor_queue_pair_CompleteInf(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    LABSTOR_INF_SPINWAIT_PREAMBLE()
    LABSTOR_INF_SPINWAIT_START()
    if(labstor_request_map_Set(&qp->cq_, rq)) {
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
    return labstor_request_map_FindAndRemove(&qp->cq_, req_id, rq);
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
    return labstor_request_queue_GetDepth(&qp->sq_);
}

static inline labstor_qid_t* labstor_queue_pair_GetQID(struct labstor_queue_pair *qp) {
    return labstor_request_queue_GetQID(&qp->sq_);
}

static inline int labstor_queue_pair_GetPID(struct labstor_queue_pair *qp) {
    return LABSTOR_GET_QP_IPC_ID(*labstor_queue_pair_GetQID(qp));
}

#ifdef __cplusplus

/*queue_pair_ptr*/

void labstor::ipc::queue_pair_ptr::Init(labstor::ipc::qid_t qid, void *sq_region, void *cq_region, void *base_region) {
    labstor_queue_pair_ptr_Init(this, qid, sq_region, cq_region, base_region);
}

/*queue_pair*/

labstor::ipc::qid_t& labstor_queue_pair::GetQID() {
    return sq_.GetQID();
}
uint32_t labstor_queue_pair::GetSize(uint32_t queue_depth) {
    return labstor_queue_pair_GetSize_global(queue_depth);
}
void labstor_queue_pair::GetPointer(labstor::ipc::queue_pair_ptr &ptr, void *base_region) {
    labstor_queue_pair_GetPointer(this, &ptr, base_region);
}
uint32_t labstor_queue_pair::GetDepth() {
    return labstor_queue_pair_GetDepth(this);
}
void labstor_queue_pair::Init(labstor::ipc::qid_t qid, void *base_region, uint32_t depth, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_queue_pair_Init(this, qid, base_region, depth, sq_region, sq_size, cq_region, cq_size);
}
void labstor_queue_pair::Init(labstor::ipc::qid_t qid, void *base_region, void *sq_region, uint32_t sq_size, void *cq_region, uint32_t cq_size) {
    labstor_queue_pair_Init(this, qid, base_region, 0, sq_region, sq_size, cq_region, cq_size);
}
void labstor_queue_pair::Attach(labstor::ipc::queue_pair_ptr &ptr, void *base_region) {
    labstor_queue_pair_Attach(this, &ptr, base_region);
}
void labstor_queue_pair::RemoteAttach(labstor::ipc::queue_pair_ptr &ptr, void *kern_base_region) {
    labstor_queue_pair_RemoteAttach(this, &ptr, kern_base_region);
}

namespace labstor::ipc {
struct shmem_queue_pair : public labstor::queue_pair, public labstor_queue_pair {
    uint32_t GetDepth() {
        return labstor_queue_pair::GetDepth();
    }

    inline labstor::ipc::qid_t& GetQID() {
        return labstor_queue_pair::GetQID();
    }
    inline bool _Enqueue(labstor::ipc::request *rq, labstor::ipc::qtok_t &qtok) {
        if(!labstor_queue_pair_Enqueue(this, reinterpret_cast<labstor::ipc::request*>(rq), &qtok)) {
            throw labstor::FAILED_TO_ENQUEUE.format();
        }
        return true;
    }
    inline bool _Peek(labstor::ipc::request **rq, int i) {
        return labstor_queue_pair_Peek(this, rq, i);
    }
    inline bool _Dequeue(labstor::ipc::request **rq) {
        return labstor_queue_pair_Dequeue(this, rq);
    }
    inline void _Complete(labstor_req_id_t req_id, labstor::ipc::request *rq) {
        if(!labstor_queue_pair_CompleteInf(this, rq)) {
            throw labstor::FAILED_TO_COMPLETE.format();
        }
    }
    inline virtual bool _IsComplete(labstor_req_id_t req_id, labstor::ipc::request **rq) {
        return labstor_queue_pair_IsComplete(this, req_id, rq);
    }
};
}

#endif

#endif