//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef labstor_work_queue_secure_H
#define labstor_work_queue_secure_H

#include <labstor/constants/busy_wait.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/mpmc/bitmap.h>
#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/constants/debug.h>
#endif

struct labstor_work_queue_secure_header {
    uint32_t enqueued_;
    uint32_t max_depth_;
    uint16_t plug_[2];
};

struct labstor_work_queue_secure_entry {
    struct labstor_queue_pair *qp_;
    struct labstor_credentials *creds_;
};

#ifdef __cplusplus
struct labstor_work_queue_secure : public labstor::shmem_type {
#else
struct labstor_work_queue_secure {
#endif
    struct labstor_work_queue_secure_header *header_;
    struct labstor_work_queue_secure_entry *queue_;
#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t max_depth);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0);
    inline void Attach(void *region);
    inline bool Enqueue(struct labstor_queue_pair *qp, struct labstor_credentials *creds);
    inline bool Peek(struct labstor_queue_pair *&qp, struct labstor_credentials *&creds, int i);
    inline uint32_t GetDepth();
    inline uint32_t GetMaxDepth();
#endif
};

static inline uint32_t labstor_work_queue_secure_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_work_queue_secure_header) +
            sizeof(labstor_work_queue_secure_entry)*max_depth;
}

static inline uint32_t labstor_work_queue_secure_GetSize(struct labstor_work_queue_secure *rbuf) {
    return labstor_work_queue_secure_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_work_queue_secure_GetRegion(struct labstor_work_queue_secure *rbuf) {
    return rbuf->header_;
}

static inline void* labstor_work_queue_secure_GetNextSection(struct labstor_work_queue_secure *rbuf) {
    return (char*)rbuf->header_ + labstor_work_queue_secure_GetSize(rbuf);
}

static inline uint32_t labstor_work_queue_secure_GetDepth(struct labstor_work_queue_secure *rbuf) {
    return rbuf->header_->enqueued_;
}

static inline uint32_t labstor_work_queue_secure_GetMaxDepth(struct labstor_work_queue_secure *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_work_queue_secure_Init(struct labstor_work_queue_secure *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    rbuf->header_ = (struct labstor_work_queue_secure_header*)region;
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->plug_[0] = 0;
    rbuf->header_->plug_[1] = 0;
    if(region_size < labstor_work_queue_secure_GetSize_global(max_depth)) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    if(max_depth == 0) {
        max_depth = region_size - sizeof(struct labstor_work_queue_secure_header);
        max_depth /= sizeof(struct labstor_work_queue_secure_entry);
    }
    if(max_depth ==0) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    rbuf->header_->max_depth_ = max_depth;
    rbuf->queue_ = (struct labstor_work_queue_secure_entry*)(rbuf->header_+1);
    return true;
}

static inline void labstor_work_queue_secure_Attach(struct labstor_work_queue_secure *rbuf, void *region) {
    rbuf->header_ = (struct labstor_work_queue_secure_header*)region;
    rbuf->queue_ = (struct labstor_work_queue_secure_entry*)(rbuf->header_ + 1);
}

static inline bool labstor_work_queue_secure_Enqueue(struct labstor_work_queue_secure *rbuf, struct labstor_queue_pair *qp, struct labstor_credentials *creds) {
    AUTO_TRACE("Enqueued", rbuf->header_->enqueued_, "depth", rbuf->header_->max_depth_)
    if(rbuf->header_->enqueued_ >= rbuf->header_->max_depth_) { return false; }
    rbuf->queue_[rbuf->header_->enqueued_].qp_ = qp;
    rbuf->queue_[rbuf->header_->enqueued_].creds_ = creds;
    ++rbuf->header_->enqueued_;
    return true;
}

static inline bool labstor_work_queue_secure_Peek(
        struct labstor_work_queue_secure *rbuf, struct labstor_queue_pair **qp, struct labstor_credentials **creds, int i) {
    if(i >= rbuf->header_->enqueued_) { return false; }
    *qp = rbuf->queue_[i].qp_;
    *creds = rbuf->queue_[i].creds_;
    return true;
}

#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_work_queue_secure work_queue_secure;
}

uint32_t labstor_work_queue_secure::GetSize(uint32_t max_depth) {
    return labstor_work_queue_secure_GetSize_global(max_depth);
}
uint32_t labstor_work_queue_secure::GetSize() {
    return labstor_work_queue_secure_GetSize(this);
}
void* labstor_work_queue_secure::GetRegion() {
    return labstor_work_queue_secure_GetRegion(this);
}
void labstor_work_queue_secure::Init(void *region, uint32_t region_size, uint32_t max_depth) {
    labstor_work_queue_secure_Init(this, region, region_size, max_depth);
}
void labstor_work_queue_secure::Attach(void *region) {
    labstor_work_queue_secure_Attach(this, region);
}
bool labstor_work_queue_secure::Enqueue(struct labstor_queue_pair *qp, struct labstor_credentials *creds) {
    return labstor_work_queue_secure_Enqueue(this, qp, creds);
}
bool labstor_work_queue_secure::Peek(struct labstor_queue_pair *&qp, struct labstor_credentials *&creds, int i) {
    return labstor_work_queue_secure_Peek(this, &qp, &creds, i);
}
uint32_t labstor_work_queue_secure::GetDepth() {
    return labstor_work_queue_secure_GetDepth(this);
}
uint32_t labstor_work_queue_secure::GetMaxDepth() {
    return labstor_work_queue_secure_GetMaxDepth(this);
}

#endif

#endif //labstor_work_queue_secure_H
