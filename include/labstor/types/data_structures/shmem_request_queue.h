//
// Created by lukemartinlogan on 11/12/21.
//

#ifndef LABSTOR_SHMEM_REQUEST_QUEUE_H
#define LABSTOR_SHMEM_REQUEST_QUEUE_H

#include <labstor/types/basics.h>

#ifdef KERNEL_BUILD
#include <linux/types.h>
#endif

#define LABSTOR_QUEUE_INTERMEDIATE 1
#define LABSTOR_QUEUE_UNORDERED 2
#define LABSTOR_QUEUE_INTERNAL 4
#define LABSTOR_QUEUE_BATCH 8
#define LABSTOR_QUEUE_HIGH_LATENCY 16

#define LABSTOR_QUEUE_PRIMARY 0
#define LABSTOR_QUEUE_ORDERED 0
#define LABSTOR_QUEUE_SHMEM 0
#define LABSTOR_QUEUE_STREAM
#define LABSTOR_QUEUE_LOW_LATENCY 0

#include <labstor/types/atomic_busy.h>
#include <labstor/types/data_structures/ring_buffer.h>
#include <labstor/constants/macros.h>


#ifdef __cplusplus
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace labstor {

struct request {
    uint32_t qtok_;
    uint32_t ns_id_;
};

struct request_queue_header {
    uint32_t qid_, flags_;
    AtomicBusy update_lock_;
};

class request_queue {
private:
    void *region_;
    request_queue_header *header_;
    ring_buffer<uint32_t> queue_;
    AtomicBusy *update_lock_;
public:
    void Init(void *region, uint32_t region_size, uint32_t qid, uint32_t flags) {
        region_ = region;
        header_ = (request_queue_header*)region;
        header_->qid_ = qid;
        header_->flags_ = flags;
        header_->update_lock_.Init();
        update_lock_ = &header_->update_lock_;
        queue_.Init(header_+1, region_size - sizeof(request_queue_header));
    }
    void Attach(void *region) {
        region_ = region;
        header_ = (request_queue_header*)region;
        update_lock_ = &header_->update_lock_;
        queue_.Attach(header_ + 1);
    }
    bool Enqueue(request *rq) {
        return queue_.Enqueue(LABSTOR_REGION_SUB(rq, region_));
    }
    request* Dequeue() {
        uint32_t off;
        if(!queue_.Dequeue(off)) {
            return nullptr;
        }
        return (request*)LABSTOR_REGION_ADD(off, region_);
    }

    inline void MarkPaused() {
        update_lock_->MarkPaused();
    }
    inline void IsPaused() {
        update_lock_->IsPaused();
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

struct queue_pair {
    request_queue submission;
    request_queue completion;
    queue_pair(
        void *sub_region, size_t sub_region_size,
        void *comp_region, size_t comp_region_size,
        uint32_t qid,
        uint32_t flags) {
        submission.Init(sub_region, sub_region_size, flags, qid);
        completion.Init(comp_region, comp_region_size, flags, qid);
    }
};

struct queue_pair_ptr {
    uint32_t sq_off;
    uint32_t cq_off;
};

}

#endif

struct labstor_request {
    int qid;
    size_t req_id;
    size_t next;
    size_t est_time_us;
};

struct labstor_km_request {
    struct labstor_id module_id;
};

#ifdef KERNEL_BUILD
#endif

#endif //LABSTOR_SHMEM_REQUEST_QUEUE_H
