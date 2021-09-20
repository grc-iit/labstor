//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

#ifdef __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <labstor/ipc/simple_allocator.h>

namespace labstor::ipc {

struct request {
    size_t next;
    size_t est_time_us;
    bool occupied;
};

struct request_queue_header {
    size_t region_size;
    size_t enqueued_, dequeued_;
    size_t head_, tail_;
};

struct request_queue {
    size_t region_size;
    void *region_;
    struct request_queue_header *header_;
    struct simple_allocator allocator_;

    request_queue() {}
    request_queue(void *region, size_t region_size, size_t request_unit) {
        Init(region, region_size, request_unit);
    }

    void Init(void *region, size_t region_size, size_t request_unit) {
        memset(region, 0, region_size);
        region_ = region;
        header_ = (struct request_queue_header*)region;
        header_->region_size = region_size;
        allocator_.Init((void*)(header_ + 1), region_size - sizeof(struct request_queue_header), request_unit);
    }

    void Attach(void *region) {
        region_ = region;
        header_ = (struct request_queue_header*)region;
        region_size = header_->region_size;
        allocator_.Attach((void*)(header_ + 1), region_size - sizeof(struct request_queue_header));
    }

    struct request* Allocate(size_t size) {
        return (struct request*)allocator_.Alloc(size);
    }

    inline bool Enqueue(struct request *rq, size_t est_time_us = 1) {
        rq->est_time_us = est_time_us;
        rq->occupied = true;
        if(header_->enqueued_ == header_->dequeued_) {
            header_->tail_ = (size_t)rq - (size_t)region_;
            header_->head_ = header_->tail_;
        }
        ++header_->enqueued_;
        return true;
    }

    inline struct request* Dequeue() {
        if(header_->enqueued_ == header_->dequeued_) {
            return nullptr;
        }
        struct request *rq = (struct request *)((size_t)region_ + (size_t)header_->head_);
        ++header_->dequeued_;
        return rq;
    }

    inline void Free(void *rq_data) {
        allocator_.Free(rq_data);
    }
};

struct queue_pair {
    request_queue submission;
    request_queue completion;
};

}

#endif

#endif //LABSTOR_REQUEST_QUEUE_H
