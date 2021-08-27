//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

namespace labstor::ipc {

struct request_queue_header {
    int queue_len_;
    int head_, tail_;
    T *queue_;
};

template<typename T>
struct request_queue {
    pid_t pid_;
    struct request_queue_header *header_;

    request_queue() : pid_(0) {}

    inline void create_queue(void *region, int queue_len) {
    }

    inline void open_queue(void *region) {
    }

    inline T *get_queue() {
        return static_cast<T>(queue_.get_address());
    }
    inline void enqueue(void *rq) {
        if(tail_ >= queue_len) {
            tail_ = 0;
        }
        if(head_ >= queue_len) {
            head_ = 0;
        }
        //How do I know when queue is full?
        get_queue()[tail_++] = rq;
    }
    inline T *dequeue() {
        if(head_ >= queue_len) {
            head_ = 0;
        }
        return &get_queue()[head_++];
    }
};

struct queue_pair {
    struct request_queue submission;
    struct request_queue completion;
};

#endif //LABSTOR_REQUEST_QUEUE_H
