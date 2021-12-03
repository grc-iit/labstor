//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

#include <labstor/kernel/constants/macros.h>
#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_off_t.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#include <labstor/types/data_structures/shmem_request.h>

struct labstor_request_queue_header {
    labstor_qid_t qid_;
    uint32_t update_lock_;
};

struct labstor_request_queue {
    struct labstor_request_queue_header *header_;
    struct labstor_ring_buffer_off_t queue_;
    struct labstor_atomic_busy update_lock_;
};

static inline uint32_t labstor_request_queue_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_request_queue_header) + labstor_ring_buffer_off_t_GetSize_global(max_depth);
}

static inline uint32_t labstor_request_queue_GetSize(struct labstor_request_queue *lrq) {
    return labstor_request_queue_GetSize_global(labstor_ring_buffer_off_t_GetMaxDepth(&lrq->queue_));
}

static inline void* labstor_request_queue_GetRegion(struct labstor_request_queue *lrq) {
    return lrq->header_;
}

static inline uint32_t labstor_request_queue_GetDepth(struct labstor_request_queue *lrq) {
    return labstor_ring_buffer_off_t_GetDepth(&lrq->queue_);
}
static inline uint32_t labstor_request_queue_GetFlags(struct labstor_request_queue *lrq) {
    return lrq->header_->qid_;
}

static inline void labstor_request_queue_Init(struct labstor_request_queue *lrq, void *region, uint32_t region_size, labstor_qid_t qid) {
    lrq->header_ = (struct labstor_request_queue_header*)region;
    lrq->header_->qid_ = qid;
    lrq->update_lock_Init(&lrq->update_lock_, &lrq->header_->update_lock_);
    labstor_ring_buffer_off_t_Init(&lrq->queue_, lrq->header_+1, region_size - sizeof(struct labstor_request_queue_header), 0);
}
static inline void labstor_request_queue_Attach(struct labstor_request_queue *lrq, void *region) {
    lrq->header_ = (struct labstor_request_queue_header*)region;
    lrq->update_lock_Attach(&lrq->update_lock_, &lrq->header_->update_lock_);
    labstor_ring_buffer_off_t_Attach(&lrq->queue_, lrq->header_ + 1);
}

static inline labstor_qid_t labstor_request_queue_GetQid(struct labstor_request_queue *lrq) {
    return lrq->header_->qid_;
}

static inline struct labstor_qtok_t labstor_request_queue_Enqueue(struct labstor_request_queue *lrq, struct labstor_request *rq) {
    struct labstor_qtok_t qtok;
    qtok.qid = lrq->header_->qid_;
    while(!labstor_ring_buffer_off_t_Enqueue(&lrq->queue_, LABSTOR_REGION_SUB(rq, lrq->header_), &rq->req_id_)) {}
    qtok.req_id = rq->req_id_;
    return qtok;
}

static inline bool labstor_request_queue_Dequeue(struct labstor_request_queue *lrq, struct labstor_request **rq) {
    labstor_off_t off;
    if(labstor_ring_buffer_off_t_Dequeue(&lrq->queue_, &off)) { return false; }
    *rq = (struct labstor_request*)(LABSTOR_REGION_ADD(off, lrq->header_));
    return true;
}

static inline uint32_t labstor_request_queue_GetDepth(struct labstor_request_queue *lrq) {
    return labstor_ring_buffer_off_t_GetDepth(&lrq->queue_);
}
static inline uint32_t labstor_request_queue_GetFlags(struct labstor_request_queue *lrq) {
    return labstor_atomic_busy_GetFlags(&lrq->update_lock_);
}
static inline void labstor_request_queue_MarkPaused(struct labstor_request_queue *lrq) {
    return labstor_atomic_busy_MarkPaused(&lrq->update_lock_);
}
static inline bool labstor_request_queue_IsPaused(struct labstor_request_queue *lrq) {
    return labstor_atomic_busy_IsPaused(&lrq->update_lock_);
}
static inline void labstor_request_queue_UnPause(struct labstor_request_queue *lrq) {
    return labstor_atomic_busy_UnPause(&lrq->update_lock_);
}
static inline void labstor_request_queue_PleaseWork(struct labstor_request_queue *lrq) {
    return labstor_atomic_busy_PleaseWork(&lrq->update_lock_);
}
static inline void labstor_request_queue_FinishWork(struct labstor_request_queue *lrq) {
    return labstor_atomic_busy_FinishWork(&lrq->update_lock_);
}


#ifdef __cplusplus
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <labstor/userspace/util/debug.h>
#include <labstor/userspace/types/shmem_type.h>

namespace labstor::ipc {

class request_queue : public labstor_request_queue, public shmem_type {
public:
    inline static uint32_t GetSize(uint32_t max_depth) {
        return labstor_request_queue_GetSize_global(max_depth);
    }
    inline uint32_t GetSize() {
        return labstor_request_queue_GetSize(this);
    }
    inline void* GetRegion() {
        return labstor_request_queue_GetRegion(this);
    }

    inline void Init(void *region, uint32_t region_size, labstor::ipc::qid_t qid) {
        return labstor_request_queue_Init(this, region, region_size, qid);
    }

    inline void Attach(void *region) {
        return labstor_request_queue_Init(this, region);
    }

    inline qid_t GetQid() {
        return labstor_request_queue_GetQid(this);
    }

    inline qtok_t Enqueue(request *rq) {
        return labstor_request_queue_Enqueue(this, rq);
    }
    inline bool Dequeue(request *&rq) {
        return labstor_request_queue_Dequeue(this, &rq);
    }

    inline uint32_t GetDepth() {
        return labstor_request_queue_GetDepth(this);
    }
    inline uint32_t GetFlags() {
        return labstor_request_queue_GetFlags(this);
    }
    inline void MarkPaused() {
        return labstor_request_queue_MarkPaused(this);
    }
    inline bool IsPaused() {
        return labstor_request_queue_IsPaused(this);
    }
    inline void UnPause() {
        return labstor_request_queue_UnPause(this);
    }
    inline void PleaseWork() {
        return labstor_request_queue_PleaseWork(this);
    }
    inline void FinishWork() {
        return labstor_request_queue_FinishWork(this);
    }
};

}

#endif


#endif //LABSTOR_REQUEST_QUEUE_H
