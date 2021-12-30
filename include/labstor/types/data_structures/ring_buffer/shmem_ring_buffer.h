//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef labstor_ring_buffer_mpmc_{T_NAME}_H
#define labstor_ring_buffer_mpmc_{T_NAME}_H

#include <labstor/constants/busy_wait.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/mpmc/bitmap.h>
#include <labstor/types/shmem_type.h>
#include <labstor/userspace/util/errors.h>

struct labstor_ring_buffer_mpmc_{T_NAME}_header {
    uint32_t enqueued_, dequeued_;
    uint16_t e_lock_, d_lock_;
    uint32_t max_depth_;
    labstor_bitmap_t bitmap_[];
};

#ifdef __cplusplus
struct labstor_ring_buffer_mpmc_{T_NAME} : public labstor::shmem_type {
#else
struct labstor_ring_buffer_mpmc_{T_NAME} {
#endif
    struct labstor_ring_buffer_mpmc_{T_NAME}_header *header_;
    labstor_bitmap_t *bitmap_;
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

static inline uint32_t labstor_ring_buffer_mpmc_{T_NAME}_GetSize_global(uint32_t max_depth) {
    return sizeof(struct labstor_ring_buffer_mpmc_{T_NAME}_header) +
            sizeof({T})*max_depth
            + labstor_bitmap_GetSize(max_depth);
}

static inline uint32_t labstor_ring_buffer_mpmc_{T_NAME}_GetSize(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    return labstor_ring_buffer_mpmc_{T_NAME}_GetSize_global(rbuf->header_->max_depth_);
}

static inline void* labstor_ring_buffer_mpmc_{T_NAME}_GetRegion(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    return rbuf->header_;
}

static inline void* labstor_ring_buffer_mpmc_{T_NAME}_GetNextSection(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    return (char*)rbuf->header_ + labstor_ring_buffer_mpmc_{T_NAME}_GetSize(rbuf);
}

static inline uint32_t labstor_ring_buffer_mpmc_{T_NAME}_GetDepth(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    uint32_t enqueued = rbuf->header_->enqueued_;
    uint32_t dequeued = rbuf->header_->dequeued_;
    if(enqueued < dequeued) { return 0; }
    return enqueued - dequeued;
}

static inline uint32_t labstor_ring_buffer_mpmc_{T_NAME}_GetMaxDepth(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    return (uint32_t)(rbuf->header_->max_depth_);
}

static inline bool labstor_ring_buffer_mpmc_{T_NAME}_Init(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf, void *region, uint32_t region_size, uint32_t max_depth) {
    rbuf->header_ = (struct labstor_ring_buffer_mpmc_{T_NAME}_header*)region;
    rbuf->header_->enqueued_ = 0;
    rbuf->header_->dequeued_ = 0;
    rbuf->header_->e_lock_ = 0;
    rbuf->header_->d_lock_ = 0;
    if(region_size < labstor_ring_buffer_mpmc_{T_NAME}_GetSize_global(max_depth)) {
#ifdef __cplusplus
        throw labstor::INVALID_RING_BUFFER_SIZE.format(region_size, max_depth);
#else
        return false;
#endif
    }
    if(max_depth == 0) {
        max_depth = region_size - sizeof(struct labstor_ring_buffer_mpmc_{T_NAME}_header);
        max_depth *= LABSTOR_BITMAP_ENTRIES_PER_BLOCK;
        max_depth /= (sizeof({T})*LABSTOR_BITMAP_ENTRIES_PER_BLOCK + sizeof(labstor_bitmap_t));
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
    rbuf->queue_ = ({T}*)(labstor_bitmap_GetNextSection(rbuf->bitmap_, rbuf->header_->max_depth_));
    return true;
}

static inline void labstor_ring_buffer_mpmc_{T_NAME}_Attach(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf, void *region) {
    rbuf->header_ = (struct labstor_ring_buffer_mpmc_{T_NAME}_header*)region;
    rbuf->bitmap_ = rbuf->header_->bitmap_;
    rbuf->queue_ = ({T}*)(labstor_bitmap_GetNextSection(rbuf->bitmap_, rbuf->header_->max_depth_));
}

static inline bool labstor_ring_buffer_mpmc_{T_NAME}_Enqueue(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf, {T} data, uint32_t *req_id) {
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

static inline bool labstor_ring_buffer_mpmc_{T_NAME}_Enqueue_simple(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf, {T} data) {
    uint32_t enqueued;
    return labstor_ring_buffer_mpmc_{T_NAME}_Enqueue(rbuf, data, &enqueued);
}

static inline bool labstor_ring_buffer_mpmc_{T_NAME}_Dequeue(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf, {T} *data) {
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

static inline bool labstor_ring_buffer_mpmc_{T_NAME}_TryEnqueueLock(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    return LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->e_lock_);
}
static inline bool labstor_ring_buffer_mpmc_{T_NAME}_TryDequeueLock(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    return LABSTOR_INF_LOCK_TRYLOCK(&rbuf->header_->d_lock_);
}
static inline void labstor_ring_buffer_mpmc_{T_NAME}_AcquireEnqueueLock(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    LABSTOR_INF_LOCK_PREAMBLE()
    LABSTOR_INF_LOCK_ACQUIRE(&rbuf->header_->e_lock_);
}
static inline void labstor_ring_buffer_mpmc_{T_NAME}_AcquireDequeueLock(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    LABSTOR_INF_LOCK_PREAMBLE();
    LABSTOR_INF_LOCK_ACQUIRE(&rbuf->header_->d_lock_);
}
static inline bool labstor_ring_buffer_mpmc_{T_NAME}_AcquireTimedEnqueueLock(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf, uint32_t max_ms) {
    LABSTOR_TIMED_LOCK_PREAMBLE();
    LABSTOR_TIMED_LOCK_ACQUIRE(&rbuf->header_->e_lock_, max_ms);
    return LABSTOR_LOCK_TIMED_OUT();
}
static inline bool labstor_ring_buffer_mpmc_{T_NAME}_AcquireTimedDequeueLock(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf, uint32_t max_ms) {
    LABSTOR_TIMED_LOCK_PREAMBLE();
    LABSTOR_TIMED_LOCK_ACQUIRE(&rbuf->header_->d_lock_, max_ms);
    return LABSTOR_LOCK_TIMED_OUT();
}
static inline void labstor_ring_buffer_mpmc_{T_NAME}_ReleaseEnqueueLock(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    LABSTOR_SPINLOCK_RELEASE(&rbuf->header_->e_lock_);
}
static inline void labstor_ring_buffer_mpmc_{T_NAME}_ReleaseDequeueLock(struct labstor_ring_buffer_mpmc_{T_NAME} *rbuf) {
    LABSTOR_SPINLOCK_RELEASE(&rbuf->header_->d_lock_);
}


#ifdef __cplusplus
namespace labstor::ipc::mpmc {
    typedef labstor_ring_buffer_mpmc_{T_NAME} ring_buffer_{T_NAME};
}

uint32_t labstor_ring_buffer_mpmc_{T_NAME}::GetSize(uint32_t max_depth) {
    return labstor_ring_buffer_mpmc_{T_NAME}_GetSize_global(max_depth);
}
uint32_t labstor_ring_buffer_mpmc_{T_NAME}::GetSize() {
    return labstor_ring_buffer_mpmc_{T_NAME}_GetSize(this);
}
void* labstor_ring_buffer_mpmc_{T_NAME}::GetRegion() {
    return labstor_ring_buffer_mpmc_{T_NAME}_GetRegion(this);
}
void labstor_ring_buffer_mpmc_{T_NAME}::Init(void *region, uint32_t region_size, uint32_t max_depth) {
    labstor_ring_buffer_mpmc_{T_NAME}_Init(this, region, region_size, max_depth);
}
void labstor_ring_buffer_mpmc_{T_NAME}::Attach(void *region) {
    labstor_ring_buffer_mpmc_{T_NAME}_Attach(this, region);
}
bool labstor_ring_buffer_mpmc_{T_NAME}::Enqueue({T} data) {
    return labstor_ring_buffer_mpmc_{T_NAME}_Enqueue_simple(this, data);
}
bool labstor_ring_buffer_mpmc_{T_NAME}::Enqueue({T} data, uint32_t &req_id) {
    return labstor_ring_buffer_mpmc_{T_NAME}_Enqueue(this, data, &req_id);
}
bool labstor_ring_buffer_mpmc_{T_NAME}::Dequeue({T} &data) {
    return labstor_ring_buffer_mpmc_{T_NAME}_Dequeue(this, &data);
}
uint32_t labstor_ring_buffer_mpmc_{T_NAME}::GetDepth() {
    return labstor_ring_buffer_mpmc_{T_NAME}_GetDepth(this);
}
uint32_t labstor_ring_buffer_mpmc_{T_NAME}::GetMaxDepth() {
    return labstor_ring_buffer_mpmc_{T_NAME}_GetMaxDepth(this);
}

bool labstor_ring_buffer_mpmc_{T_NAME}::TryEnqueueLock() {
    return labstor_ring_buffer_mpmc_{T_NAME}_TryEnqueueLock(this);
}
bool labstor_ring_buffer_mpmc_{T_NAME}::TryDequeueLock() {
    return labstor_ring_buffer_mpmc_{T_NAME}_TryDequeueLock(this);
}
void labstor_ring_buffer_mpmc_{T_NAME}::AcquireEnqueueLock() {
    labstor_ring_buffer_mpmc_{T_NAME}_AcquireEnqueueLock(this);
}
void labstor_ring_buffer_mpmc_{T_NAME}::AcquireDequeueLock() {
    labstor_ring_buffer_mpmc_{T_NAME}_AcquireDequeueLock(this);
}
bool labstor_ring_buffer_mpmc_{T_NAME}::AcquireTimedEnqueueLock(uint32_t max_ms) {
    return labstor_ring_buffer_mpmc_{T_NAME}_AcquireTimedEnqueueLock(this, max_ms);
}
bool labstor_ring_buffer_mpmc_{T_NAME}::AcquireTimedDequeueLock(uint32_t max_ms) {
    return labstor_ring_buffer_mpmc_{T_NAME}_AcquireTimedDequeueLock(this, max_ms);
}
void labstor_ring_buffer_mpmc_{T_NAME}::ReleaseEnqueueLock() {
    labstor_ring_buffer_mpmc_{T_NAME}_ReleaseEnqueueLock(this);
}
void labstor_ring_buffer_mpmc_{T_NAME}::ReleaseDequeueLock() {
    labstor_ring_buffer_mpmc_{T_NAME}_ReleaseDequeueLock(this);
}

#endif

#endif //labstor_ring_buffer_mpmc_{T_NAME}_H
