//
// Created by lukemartinlogan on 11/12/21.
//

#ifndef LABSTOR_SHMEM_REQUEST_QUEUE_H
#define LABSTOR_SHMEM_REQUEST_QUEUE_H

#include <labstor/types/basics.h>

#ifdef KERNEL_BUILD
#include <linux/types.h>
#endif

#include <labstor/types/shmem_atomic_busy.h>
#include "shmem_qtok.h"
#include "shmem_request.h"
#include "shmem_ring_buffer.h"
#include <labstor/constants/macros.h>

#ifdef __cplusplus
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <labstor/util/debug.h>
#include "labstor/types/shmem_type.h"

namespace labstor::ipc {

struct request_queue_header {
    labstor::ipc::qid_t qid_;
    labstor::credentials creds_;
    AtomicBusy update_lock_;
};

class request_queue : public shmem_type {
private:
    request_queue_header *header_;
    ring_buffer<labstor::off_t> queue_;
    AtomicBusy *update_lock_;
public:
    inline static uint32_t GetSize(uint32_t max_depth) {
        return sizeof(request_queue_header) + ring_buffer<uint32_t>::GetSize(max_depth);
    }
    inline uint32_t GetSize() {
        return GetSize(queue_.GetMaxDepth());
    }
    inline void* GetRegion() { return header_; }

    inline void Init(void *region, uint32_t region_size, labstor::ipc::qid_t qid) {
        header_ = (request_queue_header*)region;
        header_->qid_ = qid;
        header_->update_lock_.Init();
        update_lock_ = &header_->update_lock_;
        queue_.Init(header_+1, region_size - sizeof(request_queue_header));
    }
    inline void Init(void *region, uint32_t region_size, labstor::ipc::qid_t qid, labstor::credentials &creds) {
        Init(region, region_size, qid);
        header_->creds_ = creds;
    }
    inline void Attach(void *region) {
        header_ = (request_queue_header*)region;
        update_lock_ = &header_->update_lock_;
        queue_.Attach(header_ + 1);
    }

    inline labstor::credentials* GetCredentials() {
        return &header_->creds_;
    }

    inline qid_t GetQid() {
        return header_->qid_;
    }

    inline qtok_t Enqueue(request *rq) {
        qtok_t qtok;
        qtok.qid = header_->qid_;
        while(!queue_.Enqueue(LABSTOR_REGION_SUB(rq, header_), qtok.req_id)) {}
        return qtok;
    }
    inline bool Dequeue(request *&rq) {
        labstor::off_t off;
        if(!queue_.Dequeue(off)) { return false; }
        rq = reinterpret_cast<request*>(LABSTOR_REGION_ADD(off, header_));
        return true;
    }

    inline uint32_t GetDepth() {
        return queue_.GetDepth();
    }
    inline uint32_t GetFlags() {
        return header_->qid_;
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
