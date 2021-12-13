//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

#include <labstor/constants/macros.h>
#include <labstor/types/data_structures/spsc/shmem_ring_buffer_labstor_off_t.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/shmem_atomic_busy.h>

struct labstor_request_queue_header {
    labstor_qid_t qid_;
    uint16_t update_[2];
};

#ifdef __cplusplus
struct labstor_request_queue : public labstor::shmem_type {
#else
struct labstor_request_queue {
#endif
    void *base_region_;
    struct labstor_request_queue_header *header_;
    struct labstor_ring_buffer_labstor_off_t queue_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t max_depth);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void Init(void *base_region, void *region, uint32_t region_size, uint32_t depth, labstor::ipc::qid_t qid);
    inline void Init(void *base_region, void *region, uint32_t region_size, labstor::ipc::qid_t qid);
    inline void Attach(void *base_region, void *region);
    inline labstor::ipc::qid_t GetQid();
    inline bool Enqueue(labstor::ipc::request *rq, labstor::ipc::qtok_t &qtok);
    inline bool Dequeue(labstor::ipc::request *&rq);
    inline uint32_t GetDepth();
    inline uint32_t GetMaxDepth();
    inline uint32_t GetFlags();
    inline void MarkPaused();
    inline bool IsPaused();
    inline void UnPause();
#endif
};

static inline uint32_t labstor_request_queue_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_request_queue_header) + labstor_ring_buffer_labstor_off_t_GetSize_global(max_depth);
}

static inline uint32_t labstor_request_queue_GetSize(struct labstor_request_queue *lrq) {
    return labstor_request_queue_GetSize_global(labstor_ring_buffer_labstor_off_t_GetMaxDepth(&lrq->queue_));
}

static inline void* labstor_request_queue_GetRegion(struct labstor_request_queue *lrq) {
    return lrq->header_;
}

static inline uint32_t labstor_request_queue_GetDepth(struct labstor_request_queue *lrq) {
    return labstor_ring_buffer_labstor_off_t_GetDepth(&lrq->queue_);
}

static inline uint32_t labstor_request_queue_GetMaxDepth(struct labstor_request_queue *lrq) {
    return labstor_ring_buffer_labstor_off_t_GetMaxDepth(&lrq->queue_);
}

static inline labstor_qid_t labstor_request_queue_GetFlags(struct labstor_request_queue *lrq) {
    return lrq->header_->qid_;
}

static inline void labstor_request_queue_Init(
        struct labstor_request_queue *lrq, void *base_region, void *region,
        uint32_t region_size, uint32_t depth, labstor_qid_t qid) {
    lrq->base_region_ = base_region;
    lrq->header_ = (struct labstor_request_queue_header*)region;
    lrq->header_->qid_ = qid;
    lrq->header_->update_[0] = 0;
    lrq->header_->update_[1] = 0;
    labstor_ring_buffer_labstor_off_t_Init(&lrq->queue_, lrq->header_+1, region_size - sizeof(struct labstor_request_queue_header), depth);
}

static inline void labstor_request_queue_Attach(struct labstor_request_queue *lrq, void *base_region, void *region) {
    lrq->base_region_ = base_region;
    lrq->header_ = (struct labstor_request_queue_header*)region;
    labstor_ring_buffer_labstor_off_t_Attach(&lrq->queue_, lrq->header_ + 1);
}

static inline labstor_qid_t labstor_request_queue_GetQid(struct labstor_request_queue *lrq) {
    return lrq->header_->qid_;
}

static inline bool labstor_request_queue_Enqueue(struct labstor_request_queue *lrq, struct labstor_request *rq, struct labstor_qtok_t *qtok) {
    LABSTOR_INF_SPINWAIT_PREAMBLE()
    LABSTOR_INF_SPINWAIT_START()
    if(labstor_ring_buffer_labstor_off_t_Enqueue(&lrq->queue_, LABSTOR_REGION_SUB(rq, lrq->base_region_), &rq->req_id_)) {
        qtok->qid = lrq->header_->qid_;
        qtok->req_id = rq->req_id_;
        return true;
    }
    LABSTOR_INF_SPINWAIT_END()
    return false;
}

static inline bool labstor_request_queue_EnqueueSimple(struct labstor_request_queue *lrq, struct labstor_request *rq) {
    LABSTOR_INF_SPINWAIT_PREAMBLE()
    LABSTOR_INF_SPINWAIT_START()
    if(labstor_ring_buffer_labstor_off_t_Enqueue(&lrq->queue_, LABSTOR_REGION_SUB(rq, lrq->base_region_), &rq->req_id_)) {
        return true;
    }
    LABSTOR_INF_SPINWAIT_END()
    return false;
}

static inline bool labstor_request_queue_Dequeue(struct labstor_request_queue *lrq, struct labstor_request **rq) {
    labstor_off_t off;
    if(!labstor_ring_buffer_labstor_off_t_Dequeue(&lrq->queue_, &off)) { return false; }
    *rq = (struct labstor_request*)(LABSTOR_REGION_ADD(off, lrq->base_region_));
    return true;
}


/*Queue Plugging*/

static inline void labstor_request_queue_MarkPaused(struct labstor_request_queue *lrq) {
}
static inline bool labstor_request_queue_IsPaused(struct labstor_request_queue *lrq) {
    return false;
}
static inline void labstor_request_queue_UnPause(struct labstor_request_queue *lrq) {
}
static inline void labstor_request_queue_PleaseWork(struct labstor_request_queue *lrq) {
}
static inline void labstor_request_queue_FinishWork(struct labstor_request_queue *lrq) {
}


#ifdef __cplusplus
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <labstor/constants/debug.h>
#include <labstor/types/shmem_type.h>

namespace labstor::ipc {
    typedef labstor_request_queue request_queue;
}

uint32_t labstor_request_queue::GetSize(uint32_t max_depth) {
    return labstor_request_queue_GetSize_global(max_depth);
}
uint32_t labstor_request_queue::GetSize() {
    return labstor_request_queue_GetSize(this);
}
void* labstor_request_queue::GetRegion() {
    return labstor_request_queue_GetRegion(this);
}
void labstor_request_queue::Init(void *base_region, void *region, uint32_t region_size, uint32_t depth, labstor::ipc::qid_t qid) {
    return labstor_request_queue_Init(this, base_region, region, region_size, depth,  qid);
}
void labstor_request_queue::Init(void *base_region, void *region, uint32_t region_size, labstor::ipc::qid_t qid) {
    return labstor_request_queue_Init(this, base_region, region, region_size, 0,  qid);
}
void labstor_request_queue::Attach(void *base_region, void *region) {
    return labstor_request_queue_Attach(this, base_region, region);
}
labstor::ipc::qid_t labstor_request_queue::GetQid() {
    return labstor_request_queue_GetQid(this);
}
bool labstor_request_queue::Enqueue(labstor::ipc::request *rq, labstor::ipc::qtok_t &qtok) {
    return labstor_request_queue_Enqueue(this, rq, &qtok);
}
bool labstor_request_queue::Dequeue(labstor::ipc::request *&rq) {
    return labstor_request_queue_Dequeue(this, reinterpret_cast<struct labstor_request **>(&rq));
}
uint32_t labstor_request_queue::GetDepth() {
    return labstor_request_queue_GetDepth(this);
}
uint32_t labstor_request_queue::GetMaxDepth() {
    return labstor_request_queue_GetMaxDepth(this);
}
uint32_t labstor_request_queue::GetFlags() {
    return labstor_request_queue_GetFlags(this);
}
void labstor_request_queue::MarkPaused() {
    return labstor_request_queue_MarkPaused(this);
}
bool labstor_request_queue::IsPaused() {
    return labstor_request_queue_IsPaused(this);
}
void labstor_request_queue::UnPause() {
    return labstor_request_queue_UnPause(this);
}

#endif


#endif //LABSTOR_REQUEST_QUEUE_H
