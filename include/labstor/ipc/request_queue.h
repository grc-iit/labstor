//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

#include <labstor/ipc/simple_allocator.h>
#include <labstor/types/basics.h>

#ifdef KERNEL_BUILD
#include <linux/types.h>
#endif

#define RQ_QUEUE_PAUSED_SERVER 1
#define RQ_QUEUE_BUSY_SERVER 2
#define RQ_QUEUE_PAUSED_CLIENT 4
#define RQ_QUEUE_BUSY_CLIENT 8

struct labstor_request {
    int qid;
    size_t req_id;
    size_t next;
    size_t est_time_us;
};

struct labstor_request_queue_header {
    int flags;
    size_t region_size_;
    size_t enqueued_, dequeued_;
    size_t head_, tail_;
};

struct labstor_request_queue {
    size_t region_size_;
    void *region_;
    struct labstor_request_queue_header *header_;
    struct labstor_simple_allocator allocator_;
};

struct labstor_queue_pair {
    struct labstor_request_queue *submission;
    struct labstor_request_queue *completion;
};

struct labstor_km_request {
    struct labstor_id module_id;
};

#ifdef __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace labstor::ipc {

struct request {
    int qid;
    size_t id;
    size_t next;
    size_t est_time_us;
};

struct request_queue_header {
    int flags, qid;
    size_t region_size_;
    size_t enqueued_, dequeued_;
    size_t head_, tail_;
};

struct request_queue {
    size_t region_size_;
    void *region_;
    struct request_queue_header *header_;
    struct labstor_simple_allocator allocator_;
    int qid;

    request_queue() {}
    request_queue(void *region, size_t region_size, size_t request_unit) {
        Init(region, region_size, request_unit);
    }

    static size_t MinimumRegionSize() {
        return sizeof(struct request_queue_header) + labstor_simple_allocator::MinimumRegionSize();
    }

    void Init(void *region, size_t region_size, size_t request_unit) {
        memset(region, 0, region_size);
        region_ = region;
        region_size_ = region_size;
        header_ = (struct request_queue_header*)region;
        header_->region_size_ = region_size;
        header_->flags = 0;
        allocator_.Init((void*)(header_ + 1), region_size - sizeof(struct request_queue_header), request_unit);
    }

    void Attach(void *region) {
        region_ = region;
        header_ = (struct request_queue_header*)region;
        region_size_ = header_->region_size_;
        allocator_.Attach((void*)(header_ + 1), region_size_ - sizeof(struct request_queue_header));
    }

    struct request* Allocate(size_t size) {
        return (struct request*)allocator_.Alloc(size);
    }

    inline bool Enqueue(struct request *rq, size_t est_time_us = 1) {
        struct request *tail;
        rq->est_time_us = est_time_us;
        rq->next = 0;
        if(header_->enqueued_ == header_->dequeued_) {
            header_->tail_ = (size_t)rq - (size_t)region_;
            header_->head_ = header_->tail_;
        } else {
            tail = (struct request*)(header_->tail_ + (size_t)region_);
            tail->next = (size_t)rq - (size_t)region_;
            header_->tail_ = tail->next;
        }
        ++header_->enqueued_;
        return true;
    }
    inline bool Enqueue(struct labstor_request *rq, size_t est_time_us = 1) {
        return Enqueue((struct request*)rq, est_time_us);
    }

    inline struct request* Dequeue() {
        if(header_->enqueued_ == header_->dequeued_) {
            return nullptr;
        }
        struct request *rq = (struct request *)((size_t)region_ + (size_t)header_->head_);
        header_->head_ = rq->next;
        ++header_->dequeued_;
        return rq;
    }
    inline struct labstor_request* DequeueC() {
        return (struct labstor_request*)Dequeue();
    }

    inline void Free(void *rq_data) {
        allocator_.Free(rq_data);
    }
};

struct queue_pair {
    request_queue submission;
    request_queue completion;
    queue_pair(
        void *sub_region, size_t sub_region_size, size_t sub_request_unit,
        void *comp_region, size_t comp_region_size, size_t comp_request_unit) {
        submission.Init(sub_region, sub_region_size, sub_request_unit);
        completion.Init(comp_region, comp_region_size, comp_request_unit);
    }
};

}

#endif

#ifdef KERNEL_BUILD

typedef void (*process_request_fn_type)(struct labstor_queue_pair *qp, void *request);
typedef void (*process_request_fn_netlink_type)(int pid, void *request);

void labstor_request_queue_init(struct labstor_request_queue *q, void *region, size_t region_size, size_t request_unit);
void labstor_request_queue_attach(struct labstor_request_queue *q, void *region);
struct labstor_request* labstor_request_queue_allocate(struct labstor_request_queue *q, size_t size);

static inline bool labstor_request_queue_enqueue(struct labstor_request_queue *q, struct labstor_request *rq, size_t est_time_us) {
    struct labstor_request *tail;
    rq->est_time_us = est_time_us;
    rq->next = 0;
    if(q->header_->enqueued_ == q->header_->dequeued_) {
        q->header_->tail_ = (size_t)rq - (size_t)q->region_;
        q->header_->head_ = q->header_->tail_;
    } else {
        tail = (struct labstor_request*)(q->header_->tail_ + (size_t)q->region_);
        tail->next = (size_t)rq - (size_t)q->region_;
        q->header_->tail_ = tail->next;
    }
    ++q->header_->enqueued_;
    return true;
}

static inline struct labstor_request* labstor_request_queue_dequeue(struct labstor_request_queue *q) {
    struct labstor_request *rq;
    if(q->header_->enqueued_ == q->header_->dequeued_) {
        return NULL;
    }
    rq = (struct labstor_request *)((size_t)q->region_ + (size_t)q->header_->head_);
    q->header_->head_ = rq->next;
    ++q->header_->dequeued_;
    return rq;
}

static inline void labstor_request_queue_free(struct labstor_request_queue *q, void *rq_data) {
    labstor_allocator_free(&q->allocator_, rq_data);
}

#endif

#endif //LABSTOR_REQUEST_QUEUE_H
