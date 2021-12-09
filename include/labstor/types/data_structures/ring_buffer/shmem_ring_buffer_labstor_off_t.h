//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef labstor_ring_buffer_labstor_off_t_H
#define labstor_ring_buffer_labstor_off_t_H

#include <labstor/types/basics.h>
#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#endif

//labstor_off_t: The semantic name of the type
//labstor_off_t: The type being buffered
//{NullValue}: The empty key to indiciate ring buffer still being modified
//{IsNull}: The empty key to indiciate ring buffer still being modified

#define MAX_TICK 10000

struct labstor_ring_buffer_labstor_off_t_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
};

#ifdef __cplusplus
struct labstor_ring_buffer_labstor_off_t : public labstor::shmem_type {
#else
struct labstor_ring_buffer_labstor_off_t {
#endif
    struct labstor_ring_buffer_labstor_off_t_header *header_;
    labstor_off_t *queue_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t max_depth);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0);
    inline void Attach(void *region);
    inline bool Enqueue(labstor_off_t data);
    inline bool Enqueue(labstor_off_t data, uint32_t &req_id);
    inline bool Dequeue(labstor_off_t &data);
    inline uint32_t GetDepth();
    inline uint32_t GetMaxDepth();
#endif
};

static inline uint32_t labstor_ring_buffer_labstor_off_t_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_ring_buffer_labstor_off_t_header) + sizeof(labstor_off_t)*max_depth;
}

static inline uint32_t labstor_ring_buffer_labstor_off_t_GetSize(struct labstor_ring_buffer_labstor_off_t *rbuf) {
    return labstor_ring_buffer_labstor_off_t_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_ring_buffer_labstor_off_t_GetRegion(struct labstor_ring_buffer_labstor_off_t *rbuf) {
    return rbuf->header_;
}

static inline void* labstor_ring_buffer_labstor_off_t_GetNextSection(struct labstor_ring_buffer_labstor_off_t *rbuf) {
    return (char*)rbuf->header_ + labstor_ring_buffer_labstor_off_t_GetSize(rbuf);
}

static inline uint32_t labstor_ring_buffer_labstor_off_t_GetDepth(struct labstor_ring_buffer_labstor_off_t *rbuf) {
    return (uint32_t)(rbuf->header_->enqueued_ - rbuf->header_->dequeued_);
}

static inline uint32_t labstor_ring_buffer_labstor_off_t_GetMaxDepth(struct labstor_ring_buffer_labstor_off_t *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_ring_buffer_labstor_off_t_Init(struct labstor_ring_buffer_labstor_off_t *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    uint32_t min_region_size;
    rbuf->header_ = (struct labstor_ring_buffer_labstor_off_t_header*)region;
    rbuf->queue_ = (labstor_off_t*)(rbuf->header_ + 1);
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->dequeued_ = 0;
    if(max_depth > 0) {
        min_region_size = labstor_ring_buffer_labstor_off_t_GetSize_global(max_depth);
        if(min_region_size > region_size) {
            //pr_warn("labstor_ring_buffer_labstor_off_t_Init: depth of %u requires at least %u bytes", max_depth, min_region_size);
            return false;
        }
        rbuf->header_->max_depth_ = max_depth;
    } else {
        rbuf->header_->max_depth_ = (region_size - sizeof(struct labstor_ring_buffer_labstor_off_t_header)) / sizeof(labstor_off_t);
    }
    return true;
}

static inline void labstor_ring_buffer_labstor_off_t_Attach(struct labstor_ring_buffer_labstor_off_t *rbuf, void *region) {
    rbuf->header_ = (struct labstor_ring_buffer_labstor_off_t_header*)region;
    rbuf->queue_ = (labstor_off_t*)(rbuf->header_ + 1);
}

static inline bool labstor_ring_buffer_labstor_off_t_Enqueue(struct labstor_ring_buffer_labstor_off_t *rbuf, labstor_off_t data, uint32_t *req_id) {
    uint64_t enqueued;
    uint32_t off;
    do {
        enqueued = rbuf->header_->enqueued_;
        if(labstor_ring_buffer_labstor_off_t_GetDepth(rbuf) > rbuf->header_->max_depth_) { return false; }
        off = (uint32_t)(enqueued % rbuf->header_->max_depth_);
        rbuf->queue_[off] = 0;
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->enqueued_, &enqueued, enqueued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    *req_id = (uint32_t)(enqueued % (1u << 31));
    rbuf->queue_[off] = data;
    return true;
}

static inline bool labstor_ring_buffer_labstor_off_t_Enqueue_simple(struct labstor_ring_buffer_labstor_off_t *rbuf, labstor_off_t data) {
    uint32_t enqueued;
    return labstor_ring_buffer_labstor_off_t_Enqueue(rbuf, data, &enqueued);
}

static inline bool labstor_ring_buffer_labstor_off_t_Dequeue(struct labstor_ring_buffer_labstor_off_t *rbuf, labstor_off_t *data) {
    uint64_t dequeued;
    uint32_t off, tick=0;
    do {
        dequeued = rbuf->header_->dequeued_;
        if(rbuf->header_->enqueued_ == dequeued) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->dequeued_, &dequeued, dequeued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    off = dequeued % rbuf->header_->max_depth_;
    do { *data = rbuf->queue_[off]; ++tick; } while(*data == 0 && tick < MAX_TICK);
    if(tick == MAX_TICK) { return false; }
    return true;
}

#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_ring_buffer_labstor_off_t ring_buffer_labstor_off_t;
}

uint32_t labstor_ring_buffer_labstor_off_t::GetSize(uint32_t max_depth) {
    return labstor_ring_buffer_labstor_off_t_GetSize_global(max_depth);
}
uint32_t labstor_ring_buffer_labstor_off_t::GetSize() {
    return labstor_ring_buffer_labstor_off_t_GetSize(this);
}
void* labstor_ring_buffer_labstor_off_t::GetRegion() {
    return labstor_ring_buffer_labstor_off_t_GetRegion(this);
}
void labstor_ring_buffer_labstor_off_t::Init(void *region, uint32_t region_size, uint32_t max_depth) {
    labstor_ring_buffer_labstor_off_t_Init(this, region, region_size, max_depth);
}
void labstor_ring_buffer_labstor_off_t::Attach(void *region) {
    labstor_ring_buffer_labstor_off_t_Attach(this, region);
}
bool labstor_ring_buffer_labstor_off_t::Enqueue(labstor_off_t data) {
    return labstor_ring_buffer_labstor_off_t_Enqueue_simple(this, data);
}
bool labstor_ring_buffer_labstor_off_t::Enqueue(labstor_off_t data, uint32_t &req_id) {
    return labstor_ring_buffer_labstor_off_t_Enqueue(this, data, &req_id);
}
bool labstor_ring_buffer_labstor_off_t::Dequeue(labstor_off_t &data) {
    return labstor_ring_buffer_labstor_off_t_Dequeue(this, &data);
}
uint32_t labstor_ring_buffer_labstor_off_t::GetDepth() {
    return labstor_ring_buffer_labstor_off_t_GetDepth(this);
}
uint32_t labstor_ring_buffer_labstor_off_t::GetMaxDepth() {
    return labstor_ring_buffer_labstor_off_t_GetMaxDepth(this);
}

#endif

#endif //labstor_ring_buffer_labstor_off_t_H
