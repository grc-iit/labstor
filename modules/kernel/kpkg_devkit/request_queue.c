//
// Created by lukemartinlogan on 9/14/21.
//

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>

#include "request_queue.h"
#include <linux/kernel.h>
#include <linux/string.h>

/*
void labstor_request_queue_init(struct labstor_request_queue *q, void *region, size_t region_size, size_t request_unit) {
    memset(region, 0, region_size);
    q->region_ = region;
    q->header_ = (struct labstor_request_queue_header*)region;
    q->region_size_ = region_size;
    q->header_->region_size_ = region_size;
    labstor_allocator_init(&q->allocator_, (void*)(q->header_ + 1), region_size - sizeof(struct labstor_request_queue_header), request_unit);
}
EXPORT_SYMBOL(labstor_request_queue_init);

void labstor_request_queue_attach(struct labstor_request_queue *q, void *region) {
    q->region_ = region;
    q->header_ = (struct labstor_request_queue_header*)region;
    q->region_size_ = q->header_->region_size_;
    labstor_allocator_attach(&q->allocator_, (void*)(q->header_ + 1), q->region_size_ - sizeof(struct labstor_request_queue_header));
}
EXPORT_SYMBOL(labstor_request_queue_attach);

struct labstor_request* labstor_request_queue_allocate(struct labstor_request_queue *q, size_t size) {
    return (struct labstor_request*)labstor_allocator_alloc(&q->allocator_, size);
}
EXPORT_SYMBOL(labstor_request_queue_allocate);
*/