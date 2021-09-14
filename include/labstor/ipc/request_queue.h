//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_REQUEST_QUEUE_H
#define LABSTOR_REQUEST_QUEUE_H

namespace labstor::ipc {

struct request_queue_header {
    int queue_len_;
    int head_, tail_;
};

struct request_queue {
    pid_t pid_;
    void *region_;
    request_queue_header *header_;
    size_t queue_len_;
    size_t req_size_;
    int flags_;
    char *queue_;

    request_queue() : pid_(0), req_size_(0) {}

    /*inline void init(void *region, size_t region_size) {
        header_ = (request_queue_header*)(region);
        queue_len_ = (region_size - sizeof(request_queue_header)) / sizeof(request<T>);
        header_->queue_len = queue_len_;
        header_->head_ = 0;
        header_->tail_ = 0;
        queue_ = (request<T>*)(header_ + 1);
    }

    void enqueue(T request) {
        request<T> *req = queue_ + header_->tail_;
        req->data_ = request;
        header_->tail_ = (header_->tail_ + 1)%queue_len_;
    }

    request<T>* dequeue() {
        request<T> *req = queue_ + header_->head_;
        header_->head_ = (header_->head_ + 1)%queue_len_;
        return req;
    }*/
};

struct queue_pair {
    request_queue submission;
    request_queue completion;
};

typedef int qtoken;

}

#endif //LABSTOR_REQUEST_QUEUE_H
