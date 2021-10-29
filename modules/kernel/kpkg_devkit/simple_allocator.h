//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_SIMPLE_ALLOCATOR_H
#define LABSTOR_SIMPLE_ALLOCATOR_H

#include <linux/types.h>

struct simple_allocator_list_entry {
    size_t next;
};

struct simple_allocator_header {
    size_t region_size;
    size_t request_unit;
    size_t head_, tail_;
    size_t seg_off_;
};

struct simple_allocator {
    void *region_;
    struct simple_allocator_header *header_;
    char *alloc_data_;
};

static inline void labstor_allocator_init(struct simple_allocator *alloc, void *region, size_t region_size, size_t request_unit) {
    alloc->region_ = region;
    alloc->header_ = (struct simple_allocator_header*)region;
    alloc->header_->request_unit = request_unit;
    alloc->header_->region_size = region_size;
    alloc->header_->head_ = 0;
    alloc->header_->seg_off_ = 0;
    alloc->alloc_data_ = (char*)(alloc->header_ + 1);
}

static inline void labstor_allocator_attach(struct simple_allocator *alloc, void *region, size_t region_size) {
    alloc->region_ = region;
    alloc->header_ = (struct simple_allocator_header*)region;
}

static inline void* labstor_allocator_alloc(struct simple_allocator *alloc, size_t size) {
    struct simple_allocator_list_entry *entry;
    if(alloc->header_->seg_off_ < alloc->header_->region_size) {
        entry = (struct simple_allocator_list_entry *)(alloc->alloc_data_ + alloc->header_->seg_off_);
        alloc->header_->seg_off_ += alloc->header_->request_unit;
        return (void*)(entry + 1);
    }
    else if(alloc->header_->head_) {
        entry = (struct simple_allocator_list_entry *)(alloc->alloc_data_ + alloc->header_->head_);
        alloc->header_->head_ = entry->next;
        return (void*)(entry + 1);
    }
    return NULL;
}

static inline void labstor_allocator_free(struct simple_allocator *alloc, void *data) {
    struct simple_allocator_list_entry *entry = (struct simple_allocator_list_entry *)data - 1;
    struct simple_allocator_list_entry *tail = (struct simple_allocator_list_entry *)(alloc->alloc_data_ + alloc->header_->tail_);
    entry->next = 0;
    tail->next = (char*)entry - alloc->alloc_data_;
    alloc->header_->tail_ = tail->next;
    if(alloc->header_->head_ == 0) {
        alloc->header_->head_ = alloc->header_->tail_;
    }
}

#endif //LABSTOR_SIMPLE_ALLOCATOR_H
