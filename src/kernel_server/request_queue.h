//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_KERNEL_SERVER_REQUEST_QUEUE
#define LABSTOR_KERNEL_SERVER_REQUEST_QUEUE

struct request_queue_header {
    int queue_len_;
    int head_, tail_;
};

struct request_queue {
    pid_t pid_;
    void *region_;
    struct request_queue_header *header_;
    size_t queue_len_;
    size_t req_size_;
    int flags_;
    char *queue_;
};

struct queue_pair {
    struct request_queue submission_;
    struct request_queue completion_;
};

#endif //LABSTOR_KERNEL_SERVER_REQUEST_QUEUE
