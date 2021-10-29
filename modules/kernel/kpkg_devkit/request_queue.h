//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_KERNEL_SERVER_REQUEST_QUEUE
#define LABSTOR_KERNEL_SERVER_REQUEST_QUEUE

#include <labstor/types/basics.h>
#include <linux/types.h>
#include "simple_allocator.h"

struct labstor_request {
    size_t next;
    size_t est_time_us;
};

struct labstor_request_queue_header {
    size_t region_size_;
    size_t enqueued_, dequeued_;
    size_t head_, tail_;
};

struct labstor_request_queue {
    size_t region_size_;
    void *region_;
    struct labstor_request_queue_header *header_;
    struct simple_allocator allocator_;
};

struct labstor_queue_pair {
    struct labstor_request_queue *submission;
    struct labstor_request_queue *completion;
};

struct km_request {
    struct labstor_id module_id;
};

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

#endif //LABSTOR_KERNEL_SERVER_REQUEST_QUEUE
