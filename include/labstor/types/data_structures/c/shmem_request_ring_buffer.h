//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_RING_BUFFER_OFF_T_SPSC_H
#define LABSTOR_RING_BUFFER_OFF_T_SPSC_H

#include "labstor/constants/busy_wait.h"
#include "labstor/types/basics.h"
#include "labstor/types/data_structures/bitmap.h"
#ifdef __cplusplus
#include "labstor/types/shmem_type.h"
#include "labstor/userspace/util/errors.h"
#endif

//WORKER:
//Enqeued 3 reqs in a queue of size 3
//0: Peek request and process. It returns, but waits for something else to complete.
    //If ordered, go to next queue.
    //If unordered, go to next req in queue
    //Let's say it's unordered
//1: Peek request and process. It returns, but doesn't get dequeued.
//2: Peek request and process. It completes fully, freeing up an additional space to enqueue

//CLIENT:
//Enqueue request. If the head is currently occupied and there are free entries, find the next request.
//

struct LABSTOR_RING_BUFFER_OFF_T_SPSC_Header {
    uint32_t enqueued_, dequeued_;
#ifdef ENABLE_LOCKING
    uint16_t e_lock_, d_lock_;
#endif
    uint32_t max_depth_;
};

#ifdef __cplusplus
struct labstor_request_ring_buffer : public labstor::shmem_type {
#else
struct labstor_request_ring_buffer {
#endif
    struct LABSTOR_RING_BUFFER_OFF_T_SPSC_Header *header_;
    labstor_off_t *queue_;
#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t max_depth);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0);
    inline void Attach(void *region);
    inline bool Enqueue(labstor_off_t data);
    inline bool Enqueue(labstor_off_t data, uint32_t &req_id);
    inline bool Peek(labstor_off_t &data, int i);
    inline bool Dequeue(labstor_off_t &data);
    inline uint32_t GetDepth();
    inline uint32_t GetMaxDepth();
#endif
};

static inline uint32_t labstor_request_ring_buffer_GetSize_global(uint32_t max_depth) {
    return sizeof(struct LABSTOR_RING_BUFFER_OFF_T_SPSC_Header) +
            sizeof(labstor_off_t)*max_depth;
}

static inline uint32_t labstor_request_ring_buffer_GetSize(struct labstor_request_ring_buffer *rbuf) {
    return labstor_request_ring_buffer_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_request_ring_buffer_GetRegion(struct labstor_request_ring_buffer *rbuf) {
    return rbuf->header_;
}

static inline void* labstor_request_ring_buffer_GetNextSection(struct labstor_request_ring_buffer *rbuf) {
    return (char*)rbuf->header_ + labstor_request_ring_buffer_GetSize(rbuf);
}

static inline uint32_t labstor_request_ring_buffer_GetDepth(struct labstor_request_ring_buffer *rbuf) {
    return rbuf->header_->enqueued_ - rbuf->header_->dequeued_;
}

static inline uint32_t labstor_request_ring_buffer_GetMaxDepth(struct labstor_request_ring_buffer *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_request_ring_buffer_Init(struct labstor_request_ring_buffer *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    rbuf->header_ = (struct LABSTOR_RING_BUFFER_OFF_T_SPSC_Header*)region;
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->dequeued_ = 0;
#ifdef ENABLE_LOCKING
    rbuf->header_->e_lock_ = 0;
    rbuf->header_->d_lock_ = 0;
#endif
    if(region_size < labstor_request_ring_buffer_GetSize_global(max_depth)) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    if(max_depth == 0) {
        max_depth = region_size - sizeof(struct LABSTOR_RING_BUFFER_OFF_T_SPSC_Header);
        max_depth /= sizeof(labstor_off_t);
    }
    if(max_depth ==0) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    rbuf->header_->max_depth_ = max_depth;
    rbuf->queue_ = (labstor_off_t*)(rbuf->header_+1);
    return true;
}

static inline void labstor_request_ring_buffer_Attach(struct labstor_request_ring_buffer *rbuf, void *region) {
    rbuf->header_ = (struct LABSTOR_RING_BUFFER_OFF_T_SPSC_Header*)region;
    rbuf->queue_ = (labstor_off_t*)(rbuf->header_ + 1);
}

static inline void labstor_request_ring_buffer_RemoteAttach(
        struct labstor_request_ring_buffer *rbuf, void *kern_region) {
    rbuf->header_ = (struct LABSTOR_RING_BUFFER_OFF_T_SPSC_Header*)kern_region;
    rbuf->queue_ = (labstor_off_t*)(rbuf->header_ + 1);
}

static inline bool labstor_request_ring_buffer_Enqueue(struct labstor_request_ring_buffer *rbuf, labstor_off_t data, uint32_t *req_id) {
    uint32_t entry;
#ifdef ENABLE_LOCKING
    if(LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->e_lock_)) {
#endif
        if (rbuf->header_->enqueued_ - rbuf->header_->dequeued_ == rbuf->header_->max_depth_) {
            return false;
        }
        entry = rbuf->header_->enqueued_ % rbuf->header_->max_depth_;
        *req_id = rbuf->header_->enqueued_;
        rbuf->queue_[entry] = data;
        ++rbuf->header_->enqueued_;
        return true;
#ifdef ENABLE_LOCKING
    }
    return false;
#endif
}

static inline bool labstor_request_ring_buffer_Enqueue_simple(struct labstor_request_ring_buffer *rbuf, labstor_off_t data) {
    uint32_t enqueued;
    return labstor_request_ring_buffer_Enqueue(rbuf, data, &enqueued);
}

static inline bool labstor_request_ring_buffer_Peek(struct labstor_request_ring_buffer *rbuf, labstor_off_t *data, int i) {
    uint32_t entry;
#ifdef ENABLE_LOCKING
    if(LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->d_lock_)) {
#endif
    if(rbuf->header_->enqueued_ == rbuf->header_->dequeued_) { return false; }
    entry = (rbuf->header_->dequeued_ + i) % rbuf->header_->max_depth_;
    *data = rbuf->queue_[entry];
    return true;
#ifdef ENABLE_LOCKING
    }
    return false;
#endif
}

static inline bool labstor_request_ring_buffer_Dequeue(struct labstor_request_ring_buffer *rbuf, labstor_off_t *data) {
    uint32_t entry;
#ifdef ENABLE_LOCKING
    if(LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->d_lock_)) {
#endif
        if(rbuf->header_->enqueued_ == rbuf->header_->dequeued_) { return false; }
        entry = rbuf->header_->dequeued_ % rbuf->header_->max_depth_;
        *data = rbuf->queue_[entry];
        ++rbuf->header_->dequeued_;
        return true;
#ifdef ENABLE_LOCKING
    }
    return false;
#endif
}


#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_request_ring_buffer ring_buffer_labstor_off_t;
}

uint32_t labstor_request_ring_buffer::GetSize(uint32_t max_depth) {
    return labstor_request_ring_buffer_GetSize_global(max_depth);
}
uint32_t labstor_request_ring_buffer::GetSize() {
    return labstor_request_ring_buffer_GetSize(this);
}
void* labstor_request_ring_buffer::GetRegion() {
    return labstor_request_ring_buffer_GetRegion(this);
}
void labstor_request_ring_buffer::Init(void *region, uint32_t region_size, uint32_t max_depth) {
    labstor_request_ring_buffer_Init(this, region, region_size, max_depth);
}
void labstor_request_ring_buffer::Attach(void *region) {
    labstor_request_ring_buffer_Attach(this, region);
}
bool labstor_request_ring_buffer::Enqueue(labstor_off_t data) {
    return labstor_request_ring_buffer_Enqueue_simple(this, data);
}
bool labstor_request_ring_buffer::Enqueue(labstor_off_t data, uint32_t &req_id) {
    return labstor_request_ring_buffer_Enqueue(this, data, &req_id);
}
bool labstor_request_ring_buffer::Peek(labstor_off_t &data, int i) {
    return labstor_request_ring_buffer_Peek(this, &data, i);
}
bool labstor_request_ring_buffer::Dequeue(labstor_off_t &data) {
    return labstor_request_ring_buffer_Dequeue(this, &data);
}
uint32_t labstor_request_ring_buffer::GetDepth() {
    return labstor_request_ring_buffer_GetDepth(this);
}
uint32_t labstor_request_ring_buffer::GetMaxDepth() {
    return labstor_request_ring_buffer_GetMaxDepth(this);
}

#endif

#endif //LABSTOR_RING_BUFFER_OFF_T_SPSC_H
