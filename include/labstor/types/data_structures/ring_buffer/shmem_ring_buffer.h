//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef labstor_ring_buffer_{T_NAME}_H
#define labstor_ring_buffer_{T_NAME}_H

#include <labstor/types/basics.h>
#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#endif

//{T_NAME}: The semantic name of the type
//{T}: The type being buffered
//{NullValue}: The empty key to indiciate ring buffer still being modified
//{IsNull}: The empty key to indiciate ring buffer still being modified

struct labstor_ring_buffer_{T_NAME}_header {
    uint64_t enqueued_, dequeued_;
    uint32_t max_depth_;
};

#ifdef __cplusplus
struct labstor_ring_buffer_{T_NAME} : public labstor::shmem_type {
#else
struct labstor_ring_buffer_{T_NAME} {
#endif
    struct labstor_ring_buffer_{T_NAME}_header *header_;
    {T} *queue_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t max_depth);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0);
    inline void Attach(void *region);
    inline bool Enqueue({T} data);
    inline bool Enqueue({T} data, uint32_t &req_id);
    inline bool Dequeue({T} &data);
    inline uint32_t GetDepth();
    inline uint32_t GetMaxDepth();
#endif
};

static inline uint32_t labstor_ring_buffer_{T_NAME}_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_ring_buffer_{T_NAME}_header) + sizeof({T})*max_depth;
}

static inline uint32_t labstor_ring_buffer_{T_NAME}_GetSize(struct labstor_ring_buffer_{T_NAME} *rbuf) {
    return labstor_ring_buffer_{T_NAME}_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_ring_buffer_{T_NAME}_GetRegion(struct labstor_ring_buffer_{T_NAME} *rbuf) {
    return rbuf->header_;
}

static inline void* labstor_ring_buffer_{T_NAME}_GetNextSection(struct labstor_ring_buffer_{T_NAME} *rbuf) {
    return (char*)rbuf->header_ + labstor_ring_buffer_{T_NAME}_GetSize(rbuf);
}

static inline uint32_t labstor_ring_buffer_{T_NAME}_GetDepth(struct labstor_ring_buffer_{T_NAME} *rbuf) {
    return (uint32_t)(rbuf->header_->enqueued_ - rbuf->header_->dequeued_);
}

static inline uint32_t labstor_ring_buffer_{T_NAME}_GetMaxDepth(struct labstor_ring_buffer_{T_NAME} *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_ring_buffer_{T_NAME}_Init(struct labstor_ring_buffer_{T_NAME} *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    uint32_t min_region_size;
    rbuf->header_ = (struct labstor_ring_buffer_{T_NAME}_header*)region;
    rbuf->queue_ = ({T}*)(rbuf->header_ + 1);
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->dequeued_ = 0;
    if(max_depth > 0) {
        min_region_size = labstor_ring_buffer_{T_NAME}_GetSize_global(max_depth);
        if(min_region_size > region_size) {
            //pr_warn("labstor_ring_buffer_{T_NAME}_Init: depth of %u requires at least %u bytes", max_depth, min_region_size);
            return false;
        }
        rbuf->header_->max_depth_ = max_depth;
    } else {
        rbuf->header_->max_depth_ = (region_size - sizeof(struct labstor_ring_buffer_{T_NAME}_header)) / sizeof({T});
    }
    return true;
}

static inline void labstor_ring_buffer_{T_NAME}_Attach(struct labstor_ring_buffer_{T_NAME} *rbuf, void *region) {
    rbuf->header_ = (struct labstor_ring_buffer_{T_NAME}_header*)region;
    rbuf->queue_ = ({T}*)(rbuf->header_ + 1);
}

static inline bool labstor_ring_buffer_{T_NAME}_Enqueue(struct labstor_ring_buffer_{T_NAME} *rbuf, {T} data, uint32_t *req_id) {
    uint64_t enqueued;
    uint32_t off;
    do {
        enqueued = rbuf->header_->enqueued_;
        if(labstor_ring_buffer_{T_NAME}_GetDepth(rbuf) > rbuf->header_->max_depth_) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->enqueued_, &enqueued, enqueued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    *req_id = (uint32_t)(enqueued % (1u << 31));
    off = (*req_id) % rbuf->header_->max_depth_;
    rbuf->queue_[off] = data;
    return true;
}

static inline bool labstor_ring_buffer_{T_NAME}_Enqueue_simple(struct labstor_ring_buffer_{T_NAME} *rbuf, {T} data) {
    uint32_t enqueued;
    return labstor_ring_buffer_{T_NAME}_Enqueue(rbuf, data, &enqueued);
}

static inline bool labstor_ring_buffer_{T_NAME}_Dequeue(struct labstor_ring_buffer_{T_NAME} *rbuf, {T} *data) {
    uint64_t dequeued;
    uint32t_t off;
    do {
        dequeued = rbuf->header_->dequeued_;
        if(rbuf->header_->enqueued_ == dequeued) { return false; }
    }
    while(!__atomic_compare_exchange_n(&rbuf->header_->dequeued_, &dequeued, dequeued + 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    off = dequeued % rbuf->header_->max_depth_;
    *data = rbuf->queue_[off];
    return true;
}

#ifdef __cplusplus
namespace labstor::ipc {
    typedef labstor_ring_buffer_{T_NAME} ring_buffer_{T_NAME};
}

uint32_t labstor_ring_buffer_{T_NAME}::GetSize(uint32_t max_depth) {
    return labstor_ring_buffer_{T_NAME}_GetSize_global(max_depth);
}
uint32_t labstor_ring_buffer_{T_NAME}::GetSize() {
    return labstor_ring_buffer_{T_NAME}_GetSize(this);
}
void* labstor_ring_buffer_{T_NAME}::GetRegion() {
    return labstor_ring_buffer_{T_NAME}_GetRegion(this);
}
void labstor_ring_buffer_{T_NAME}::Init(void *region, uint32_t region_size, uint32_t max_depth) {
    labstor_ring_buffer_{T_NAME}_Init(this, region, region_size, max_depth);
}
void labstor_ring_buffer_{T_NAME}::Attach(void *region) {
    labstor_ring_buffer_{T_NAME}_Attach(this, region);
}
bool labstor_ring_buffer_{T_NAME}::Enqueue({T} data) {
    return labstor_ring_buffer_{T_NAME}_Enqueue_simple(this, data);
}
bool labstor_ring_buffer_{T_NAME}::Enqueue({T} data, uint32_t &req_id) {
    return labstor_ring_buffer_{T_NAME}_Enqueue(this, data, &req_id);
}
bool labstor_ring_buffer_{T_NAME}::Dequeue({T} &data) {
    return labstor_ring_buffer_{T_NAME}_Dequeue(this, &data);
}
uint32_t labstor_ring_buffer_{T_NAME}::GetDepth() {
    return labstor_ring_buffer_{T_NAME}_GetDepth(this);
}
uint32_t labstor_ring_buffer_{T_NAME}::GetMaxDepth() {
    return labstor_ring_buffer_{T_NAME}_GetMaxDepth(this);
}

#endif

#endif //labstor_ring_buffer_{T_NAME}_H
