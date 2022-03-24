//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef labstor_work_queue_H
#define labstor_work_queue_H

#include "labstor/constants/busy_wait.h"
#include "labstor/types/basics.h"
#include "labstor/types/data_structures/bitmap.h"
#ifdef __cplusplus
#include "labstor/types/shmem_type.h"
#include "labstor/userspace/util/errors.h"
#endif

struct labstor_work_queue_header {
    uint32_t enqueued_;
    uint32_t max_depth_;
    uint16_t plug_[2];
};

#ifdef __cplusplus
struct labstor_work_queue : public labstor::shmem_type {
#else
struct labstor_work_queue {
#endif
    struct labstor_work_queue_header *header_;
    struct labstor_queue_pair **queue_;
#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t max_depth);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0);
    inline void Attach(void *region);
    inline bool Enqueue(struct labstor_queue_pair *qp);
    inline bool Peek(struct labstor_queue_pair *&qp, int i);
    inline uint32_t GetDepth();
    inline uint32_t GetMaxDepth();
#endif
};

static inline uint32_t labstor_work_queue_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_work_queue_header) +
            sizeof(labstor_off_t)*max_depth;
}

static inline uint32_t labstor_work_queue_GetSize(struct labstor_work_queue *rbuf) {
    return labstor_work_queue_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_work_queue_GetRegion(struct labstor_work_queue *rbuf) {
    return rbuf->header_;
}

static inline void* labstor_work_queue_GetNextSection(struct labstor_work_queue *rbuf) {
    return (char*)rbuf->header_ + labstor_work_queue_GetSize(rbuf);
}

static inline uint32_t labstor_work_queue_GetDepth(struct labstor_work_queue *rbuf) {
    return rbuf->header_->enqueued_;
}

static inline uint32_t labstor_work_queue_GetMaxDepth(struct labstor_work_queue *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_work_queue_Init(struct labstor_work_queue *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    rbuf->header_ = (struct labstor_work_queue_header*)region;
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->plug_[0] = 0;
    rbuf->header_->plug_[1] = 0;
    if(region_size < labstor_work_queue_GetSize_global(max_depth)) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    if(max_depth == 0) {
        max_depth = region_size - sizeof(struct labstor_work_queue_header);
        max_depth /= sizeof(struct queue_pair*);
    }
    if(max_depth ==0) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    rbuf->header_->max_depth_ = max_depth;
    rbuf->queue_ = (struct labstor_queue_pair**)(rbuf->header_+1);
    return true;
}

static inline void labstor_work_queue_Attach(struct labstor_work_queue *rbuf, void *region) {
    rbuf->header_ = (struct labstor_work_queue_header*)region;
    rbuf->queue_ = (struct labstor_queue_pair**)(rbuf->header_ + 1);
}

static inline bool labstor_work_queue_Enqueue(struct labstor_work_queue *rbuf, struct labstor_queue_pair *qp) {
    if(rbuf->header_->enqueued_ >= rbuf->header_->max_depth_) { return false; }
    rbuf->queue_[rbuf->header_->enqueued_] = qp;
    ++rbuf->header_->enqueued_;
    return true;
}

static inline bool labstor_work_queue_Peek(struct labstor_work_queue *rbuf, struct labstor_queue_pair **qp, int i) {
    if(i >= rbuf->header_->enqueued_) { return false; }
    *qp = rbuf->queue_[i];
    return true;
}


#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_work_queue work_queue;
}

uint32_t labstor_work_queue::GetSize(uint32_t max_depth) {
    return labstor_work_queue_GetSize_global(max_depth);
}
uint32_t labstor_work_queue::GetSize() {
    return labstor_work_queue_GetSize(this);
}
void* labstor_work_queue::GetRegion() {
    return labstor_work_queue_GetRegion(this);
}
void labstor_work_queue::Init(void *region, uint32_t region_size, uint32_t max_depth) {
    labstor_work_queue_Init(this, region, region_size, max_depth);
}
void labstor_work_queue::Attach(void *region) {
    labstor_work_queue_Attach(this, region);
}
bool labstor_work_queue::Enqueue(struct labstor_queue_pair *qp) {
    return labstor_work_queue_Enqueue(this, qp);
}
bool labstor_work_queue::Peek(struct labstor_queue_pair *&qp, int i) {
    return labstor_work_queue_Peek(this, &qp, i);
}
uint32_t labstor_work_queue::GetDepth() {
    return labstor_work_queue_GetDepth(this);
}
uint32_t labstor_work_queue::GetMaxDepth() {
    return labstor_work_queue_GetMaxDepth(this);
}

#endif

#endif //labstor_work_queue_H
