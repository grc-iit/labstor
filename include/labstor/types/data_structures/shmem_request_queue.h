//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

#include <labstor/constants/macros.h>
#include <labstor/types/data_structures/ring_buffer/shmem_ring_buffer_labstor_off_t.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/shmem_atomic_busy.h>

struct labstor_request_queue_header {
    labstor_qid_t qid_;
    uint32_t update_lock_;
};

#ifdef __cplusplus
struct labstor_request_queue : public labstor::shmem_type {
#else
struct labstor_request_queue {
#endif
    void *base_region_;
    struct labstor_request_queue_header *header_;
    struct labstor_ring_buffer_labstor_off_t queue_;
    struct labstor_atomic_busy update_lock_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t max_depth);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void Init(void *base_region, void *region, uint32_t region_size, labstor::ipc::qid_t qid);
    inline void Attach(void *base_region, void *region);
    inline labstor::ipc::qid_t GetQid();
    inline labstor::ipc::qtok_t Enqueue(labstor::ipc::request *rq);
    inline bool Dequeue(labstor::ipc::request *&rq);
    inline uint32_t GetDepth();
    inline uint32_t GetMaxDepth();
    inline uint32_t GetFlags();
    inline void MarkPaused();
    inline bool IsPaused();
    inline void UnPause();
    inline void PleaseWork();
    inline void FinishWork();
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

static inline void labstor_request_queue_Init(struct labstor_request_queue *lrq, void *base_region, void *region, uint32_t region_size, labstor_qid_t qid) {
    lrq->base_region_ = base_region;
    lrq->header_ = (struct labstor_request_queue_header*)region;
    lrq->header_->qid_ = qid;
    labstor_atomic_busy_Init(&lrq->update_lock_, &lrq->header_->update_lock_);
    labstor_ring_buffer_labstor_off_t_Init(&lrq->queue_, lrq->header_+1, region_size - sizeof(struct labstor_request_queue_header), 0);
}

static inline void labstor_request_queue_Attach(struct labstor_request_queue *lrq, void *base_region, void *region) {
    lrq->base_region_ = base_region;
    lrq->header_ = (struct labstor_request_queue_header*)region;
    labstor_atomic_busy_Attach(&lrq->update_lock_, &lrq->header_->update_lock_);
    labstor_ring_buffer_labstor_off_t_Attach(&lrq->queue_, lrq->header_ + 1);
}

static inline labstor_qid_t labstor_request_queue_GetQid(struct labstor_request_queue *lrq) {
    return lrq->header_->qid_;
}

static inline struct labstor_qtok_t labstor_request_queue_Enqueue(struct labstor_request_queue *lrq, struct labstor_request *rq) {
    struct labstor_qtok_t qtok;
    qtok.qid = lrq->header_->qid_;
    while(!labstor_ring_buffer_labstor_off_t_Enqueue(&lrq->queue_, LABSTOR_REGION_SUB(rq, lrq->base_region_), &rq->req_id_)) {}
    qtok.req_id = rq->req_id_;
    return qtok;
}

static inline bool labstor_request_queue_Dequeue(struct labstor_request_queue *lrq, struct labstor_request **rq) {
    labstor_off_t off;
    if(!labstor_ring_buffer_labstor_off_t_Dequeue(&lrq->queue_, &off)) { return false; }
    *rq = (struct labstor_request*)(LABSTOR_REGION_ADD(off, lrq->base_region_));
    return true;
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
void labstor_request_queue::Init(void *base_region, void *region, uint32_t region_size, labstor::ipc::qid_t qid) {
    return labstor_request_queue_Init(this, base_region, region, region_size, qid);
}
void labstor_request_queue::Attach(void *base_region, void *region) {
    return labstor_request_queue_Attach(this, base_region, region);
}
labstor::ipc::qid_t labstor_request_queue::GetQid() {
    return labstor_request_queue_GetQid(this);
}
labstor::ipc::qtok_t labstor_request_queue::Enqueue(labstor::ipc::request *rq) {
    return labstor_request_queue_Enqueue(this, rq);
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
void labstor_request_queue::PleaseWork() {
    return labstor_request_queue_PleaseWork(this);
}
void labstor_request_queue::FinishWork() {
    return labstor_request_queue_FinishWork(this);
}

#endif


#endif //LABSTOR_REQUEST_QUEUE_H
