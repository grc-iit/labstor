//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef labstor_ring_buffer_mpmc_uint32_t_H
#define labstor_ring_buffer_mpmc_uint32_t_H

#include <labstor/constants/busy_wait.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/mpmc/bitmap.h>
#ifdef __cplusplus
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>
#endif

//uint32_t: The semantic name of the type
//uint32_t: The type being buffered

struct labstor_ring_buffer_mpmc_uint32_t_header {
    uint32_t enqueued_, dequeued_;
    uint16_t e_lock_, d_lock_;
    uint32_t max_depth_;
    labstor_bitmap_t bitmap_[];
};

#ifdef __cplusplus
struct labstor_ring_buffer_mpmc_uint32_t : public labstor::shmem_type {
#else
struct labstor_ring_buffer_mpmc_uint32_t {
#endif
    struct labstor_ring_buffer_mpmc_uint32_t_header *header_;
    labstor_bitmap_t *bitmap_;
    uint32_t *queue_;

#ifdef __cplusplus
    static inline uint32_t GetSize(uint32_t max_depth);
    inline uint32_t GetSize();
    inline void* GetRegion();
    inline void Init(void *region, uint32_t region_size, uint32_t max_depth = 0);
    inline void Attach(void *region);
    inline bool Enqueue(uint32_t data);
    inline bool Enqueue(uint32_t data, uint32_t &req_id);
    inline bool Dequeue(uint32_t &data);
    inline uint32_t GetDepth();
    inline uint32_t GetMaxDepth();

    inline bool TryEnqueueLock();
    inline bool TryDequeueLock();
    inline void AcquireEnqueueLock();
    inline void AcquireDequeueLock();
    inline bool AcquireTimedEnqueueLock(uint32_t max_ms);
    inline bool AcquireTimedDequeueLock(uint32_t max_ms);
    inline void ReleaseEnqueueLock();
    inline void ReleaseDequeueLock();
#endif
};

static inline uint32_t labstor_ring_buffer_mpmc_uint32_t_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_ring_buffer_mpmc_uint32_t_header) +
            sizeof(uint32_t)*max_depth
            + labstor_bitmap_GetSize(max_depth);
}

static inline uint32_t labstor_ring_buffer_mpmc_uint32_t_GetSize(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    return labstor_ring_buffer_mpmc_uint32_t_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_ring_buffer_mpmc_uint32_t_GetRegion(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    return rbuf->header_;
}

static inline void* labstor_ring_buffer_mpmc_uint32_t_GetNextSection(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    return (char*)rbuf->header_ + labstor_ring_buffer_mpmc_uint32_t_GetSize(rbuf);
}

static inline uint32_t labstor_ring_buffer_mpmc_uint32_t_GetDepth(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    uint32_t enqueued = rbuf->header_->enqueued_;
    uint32_t dequeued = rbuf->header_->dequeued_;
    if(enqueued < dequeued) { return 0; }
    return enqueued - dequeued;
}

static inline uint32_t labstor_ring_buffer_mpmc_uint32_t_GetMaxDepth(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_ring_buffer_mpmc_uint32_t_Init(struct labstor_ring_buffer_mpmc_uint32_t *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    rbuf->header_ = (struct labstor_ring_buffer_mpmc_uint32_t_header*)region;
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->dequeued_ = 0;
    rbuf->header_->e_lock_ = 0;
    rbuf->header_->d_lock_ = 0;
    if(region_size < labstor_ring_buffer_mpmc_uint32_t_GetSize_global(max_depth)) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    if(max_depth == 0) {
        max_depth = region_size - sizeof(struct labstor_ring_buffer_mpmc_uint32_t_header);
        max_depth *= LABSTOR_BITMAP_ENTRIES_PER_BLOCK;
        max_depth /= (sizeof(uint32_t)*LABSTOR_BITMAP_ENTRIES_PER_BLOCK + sizeof(labstor_bitmap_t));
    }
    if(max_depth ==0) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    rbuf->header_->max_depth_ = max_depth;
    rbuf->bitmap_ = rbuf->header_->bitmap_;
    labstor_bitmap_Init(rbuf->bitmap_, rbuf->header_->max_depth_);
    rbuf->queue_ = (uint32_t*)(labstor_bitmap_GetNextSection(rbuf->bitmap_, rbuf->header_->max_depth_));
    return true;
}

static inline void labstor_ring_buffer_mpmc_uint32_t_Attach(struct labstor_ring_buffer_mpmc_uint32_t *rbuf, void *region) {
    rbuf->header_ = (struct labstor_ring_buffer_mpmc_uint32_t_header*)region;
    rbuf->bitmap_ = rbuf->header_->bitmap_;
    rbuf->queue_ = (uint32_t*)(labstor_bitmap_GetNextSection(rbuf->bitmap_, rbuf->header_->max_depth_));
}

static inline bool labstor_ring_buffer_mpmc_uint32_t_Enqueue(struct labstor_ring_buffer_mpmc_uint32_t *rbuf, uint32_t data, uint32_t *req_id) {
    uint32_t enqueued, dequeued, entry;
    if(LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->e_lock_)) {
        enqueued = rbuf->header_->enqueued_;
        dequeued = rbuf->header_->dequeued_;
        if(enqueued - dequeued >= rbuf->header_->max_depth_) {
            LABSTOR_INF_LOCK_RELEASE(&rbuf->header_->e_lock_);
            return false;
        }
        entry = enqueued % rbuf->header_->max_depth_;
        *req_id = enqueued;
        rbuf->queue_[entry] = data;
        labstor_bitmap_Set(rbuf->bitmap_, entry);
        ++rbuf->header_->enqueued_;
        LABSTOR_INF_LOCK_RELEASE(&rbuf->header_->e_lock_);
        return true;
    }
    return false;
}

static inline bool labstor_ring_buffer_mpmc_uint32_t_Enqueue_simple(struct labstor_ring_buffer_mpmc_uint32_t *rbuf, uint32_t data) {
    uint32_t enqueued;
    return labstor_ring_buffer_mpmc_uint32_t_Enqueue(rbuf, data, &enqueued);
}

static inline bool labstor_ring_buffer_mpmc_uint32_t_Dequeue(struct labstor_ring_buffer_mpmc_uint32_t *rbuf, uint32_t *data) {
    uint32_t enqueued, dequeued, entry;
    if(LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->d_lock_)) {
        enqueued = rbuf->header_->enqueued_;
        dequeued = rbuf->header_->dequeued_;
        entry = dequeued % rbuf->header_->max_depth_;
        if(enqueued - dequeued == 0 || !labstor_bitmap_IsSet(rbuf->bitmap_, entry)) {
            LABSTOR_INF_LOCK_RELEASE(&rbuf->header_->d_lock_);
            return false;
        }
        *data = rbuf->queue_[entry];
        labstor_bitmap_Unset(rbuf->bitmap_, entry);
        ++rbuf->header_->dequeued_;
        LABSTOR_INF_LOCK_RELEASE(&rbuf->header_->d_lock_);
        return true;
    }
    return false;
}

/*For queue plugging*/

static inline bool labstor_ring_buffer_mpmc_uint32_t_TryEnqueueLock(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    return LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->e_lock_);
}
static inline bool labstor_ring_buffer_mpmc_uint32_t_TryDequeueLock(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    return LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->d_lock_);
}
static inline void labstor_ring_buffer_mpmc_uint32_t_AcquireEnqueueLock(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    LABSTOR_INF_LOCK_PREAMBLE()
    LABSTOR_INF_LOCK_ACQUIRE(&rbuf->header_->e_lock_);
}
static inline void labstor_ring_buffer_mpmc_uint32_t_AcquireDequeueLock(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    LABSTOR_INF_LOCK_PREAMBLE();
    LABSTOR_INF_LOCK_ACQUIRE(&rbuf->header_->d_lock_);
}
static inline bool labstor_ring_buffer_mpmc_uint32_t_AcquireTimedEnqueueLock(struct labstor_ring_buffer_mpmc_uint32_t *rbuf, uint32_t max_ms) {
    LABSTOR_TIMED_LOCK_PREAMBLE();
    LABSTOR_TIMED_LOCK_ACQUIRE(&rbuf->header_->e_lock_, max_ms);
    return LABSTOR_LOCK_TIMED_OUT();
}
static inline bool labstor_ring_buffer_mpmc_uint32_t_AcquireTimedDequeueLock(struct labstor_ring_buffer_mpmc_uint32_t *rbuf, uint32_t max_ms) {
    LABSTOR_TIMED_LOCK_PREAMBLE();
    LABSTOR_TIMED_LOCK_ACQUIRE(&rbuf->header_->d_lock_, max_ms);
    return LABSTOR_LOCK_TIMED_OUT();
}
static inline void labstor_ring_buffer_mpmc_uint32_t_ReleaseEnqueueLock(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    LABSTOR_SPINLOCK_RELEASE(&rbuf->header_->e_lock_);
}
static inline void labstor_ring_buffer_mpmc_uint32_t_ReleaseDequeueLock(struct labstor_ring_buffer_mpmc_uint32_t *rbuf) {
    LABSTOR_SPINLOCK_RELEASE(&rbuf->header_->d_lock_);
}


#ifdef __cplusplus
namespace labstor::ipc::mpmc {
    typedef labstor_ring_buffer_mpmc_uint32_t ring_buffer_uint32_t;
}

uint32_t labstor_ring_buffer_mpmc_uint32_t::GetSize(uint32_t max_depth) {
    return labstor_ring_buffer_mpmc_uint32_t_GetSize_global(max_depth);
}
uint32_t labstor_ring_buffer_mpmc_uint32_t::GetSize() {
    return labstor_ring_buffer_mpmc_uint32_t_GetSize(this);
}
void* labstor_ring_buffer_mpmc_uint32_t::GetRegion() {
    return labstor_ring_buffer_mpmc_uint32_t_GetRegion(this);
}
void labstor_ring_buffer_mpmc_uint32_t::Init(void *region, uint32_t region_size, uint32_t max_depth) {
    labstor_ring_buffer_mpmc_uint32_t_Init(this, region, region_size, max_depth);
}
void labstor_ring_buffer_mpmc_uint32_t::Attach(void *region) {
    labstor_ring_buffer_mpmc_uint32_t_Attach(this, region);
}
bool labstor_ring_buffer_mpmc_uint32_t::Enqueue(uint32_t data) {
    return labstor_ring_buffer_mpmc_uint32_t_Enqueue_simple(this, data);
}
bool labstor_ring_buffer_mpmc_uint32_t::Enqueue(uint32_t data, uint32_t &req_id) {
    return labstor_ring_buffer_mpmc_uint32_t_Enqueue(this, data, &req_id);
}
bool labstor_ring_buffer_mpmc_uint32_t::Dequeue(uint32_t &data) {
    return labstor_ring_buffer_mpmc_uint32_t_Dequeue(this, &data);
}
uint32_t labstor_ring_buffer_mpmc_uint32_t::GetDepth() {
    return labstor_ring_buffer_mpmc_uint32_t_GetDepth(this);
}
uint32_t labstor_ring_buffer_mpmc_uint32_t::GetMaxDepth() {
    return labstor_ring_buffer_mpmc_uint32_t_GetMaxDepth(this);
}

bool labstor_ring_buffer_mpmc_uint32_t::TryEnqueueLock() {
    return labstor_ring_buffer_mpmc_uint32_t_TryEnqueueLock(this);
}
bool labstor_ring_buffer_mpmc_uint32_t::TryDequeueLock() {
    return labstor_ring_buffer_mpmc_uint32_t_TryDequeueLock(this);
}
void labstor_ring_buffer_mpmc_uint32_t::AcquireEnqueueLock() {
    labstor_ring_buffer_mpmc_uint32_t_AcquireEnqueueLock(this);
}
void labstor_ring_buffer_mpmc_uint32_t::AcquireDequeueLock() {
    labstor_ring_buffer_mpmc_uint32_t_AcquireDequeueLock(this);
}
bool labstor_ring_buffer_mpmc_uint32_t::AcquireTimedEnqueueLock(uint32_t max_ms) {
    return labstor_ring_buffer_mpmc_uint32_t_AcquireTimedEnqueueLock(this, max_ms);
}
bool labstor_ring_buffer_mpmc_uint32_t::AcquireTimedDequeueLock(uint32_t max_ms) {
    return labstor_ring_buffer_mpmc_uint32_t_AcquireTimedDequeueLock(this, max_ms);
}
void labstor_ring_buffer_mpmc_uint32_t::ReleaseEnqueueLock() {
    labstor_ring_buffer_mpmc_uint32_t_ReleaseEnqueueLock(this);
}
void labstor_ring_buffer_mpmc_uint32_t::ReleaseDequeueLock() {
    labstor_ring_buffer_mpmc_uint32_t_ReleaseDequeueLock(this);
}

#endif

#endif //labstor_ring_buffer_mpmc_uint32_t_H
