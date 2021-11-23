//
// Created by lukemartinlogan on 11/12/21.
//

#ifndef LABSTOR_SHMEM_REQUEST_QUEUE_H
#define LABSTOR_SHMEM_REQUEST_QUEUE_H

#include <labstor/types/basics.h>

#ifdef KERNEL_BUILD
#include <linux/types.h>
#endif

#define LABSTOR_QP_INTERMEDIATE 0x10000000
#define LABSTOR_QP_UNORDERED 0x20000000
#define LABSTOR_QP_BATCH 0x40000000
#define LABSTOR_QP_HIGH_LATENCY 0x80000000

#define LABSTOR_QP_IS_INTERMEDIATE(flags) (flags & LABSTOR_QP_INTERMEDIATE)
#define LABSTOR_QP_IS_UNORDERED(flags) (flags & LABSTOR_QP_UNORDERED)
#define LABSTOR_QP_IS_BATCH(flags) (flags & LABSTOR_QP_BATCH)
#define LABSTOR_QP_IS_HIGH_LATENCY(flags) (flags & LABSTOR_QP_HIGH_LATENCY)

#define LABSTOR_QP_IS_PRIMARY(flags) (!(flags & LABSTOR_QP_INTERMEDIATE))
#define LABSTOR_QP_IS_ORDERED(flags) (!(flags & LABSTOR_QP_UNORDERED))
#define LABSTOR_QP_IS_STREAM(flags) (!(flags & LABSTOR_QP_BATCH))
#define LABSTOR_QP_IS_LOW_LATENCY(flags) (!(flags & LABSTOR_QP_HIGH_LATENCY))

#include <labstor/types/shmem_atomic_busy.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_ring_buffer.h>
#include <labstor/constants/macros.h>

#ifdef __cplusplus
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "labstor/types/shmem_type.h"

namespace labstor::ipc {

struct request_queue_header {
    uint32_t flags_;
    labstor::credentials creds_;
    AtomicBusy update_lock_;
};

class request_queue : public shmem_type {
private:
    request_queue_header *header_;
    ring_buffer<uint32_t> queue_;
    AtomicBusy *update_lock_;
public:
    inline static uint32_t GetSize(uint32_t max_depth) {
        return sizeof(request_queue_header) + ring_buffer<uint32_t>::GetSize(max_depth);
    }
    inline uint32_t GetSize() {
        return GetSize(queue_.GetMaxDepth());
    }

    inline void Init(void *region, uint32_t region_size, uint32_t flags) {
        region_ = region;
        header_ = (request_queue_header*)region;
        header_->flags_ = flags;
        header_->update_lock_.Init();
        update_lock_ = &header_->update_lock_;
        queue_.Init(header_+1, region_size - sizeof(request_queue_header));
    }
    inline void Init(void *region, uint32_t region_size, uint32_t flags, labstor::credentials creds) {
        Init(region, region_size, flags);
        header_->creds_ = creds;
    }
    inline void Attach(void *region) {
        region_ = region;
        header_ = (request_queue_header*)region;
        update_lock_ = &header_->update_lock_;
        queue_.Attach(header_ + 1);
    }

    inline labstor::credentials* GetCredentials() {
        return &header_->creds_;
    }

    inline bool Enqueue(request *rq) {
        return queue_.Enqueue(LABSTOR_REGION_SUB(rq, region_));
    }
    inline bool Dequeue(request *&rq) {
        uint32_t off;
        if(!queue_.Dequeue(off)) { return false; }
        rq = (request*)LABSTOR_REGION_ADD(off, region_);
        return true;
    }

    inline uint32_t GetDepth() {
        return queue_.GetDepth();
    }
    inline uint32_t GetFlags() {
        return header_->flags_;
    }
    inline void MarkPaused() {
        update_lock_->MarkPaused();
    }
    inline bool IsPaused() {
        return update_lock_->IsPaused();
    }
    inline void UnPause() {
        update_lock_->UnPause();
    }
    inline void PleaseWork() {
        update_lock_->PleaseWork();
    }
    inline void FinishWork() {
        update_lock_->FinishWork();
    }
};

}

#endif

struct labstor_request {
    uint32_t qtok_;
    uint32_t ns_id_;
};

struct labstor_km_request {
    struct labstor_id module_id;
};

#ifdef KERNEL_BUILD
#endif

#endif //LABSTOR_SHMEM_REQUEST_QUEUE_H
