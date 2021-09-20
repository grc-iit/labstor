//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_SIMPLE_ALLOCATOR_H
#define LABSTOR_SIMPLE_ALLOCATOR_H

#ifdef __cplusplus

#define MAX_CACHED_SIZES

namespace labstor {

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

    inline void Init(void *region, size_t region_size, size_t request_unit) {
        header_ = (struct simple_allocator_header*)region;
        header_->request_unit = request_unit;
        header_->region_size = region_size;
        header_->head_ = 0;
        header_->seg_off_ = 0;
    }

    inline void Attach(void *region, size_t region_size) {
        header_ = (struct simple_allocator_header*)region;
    }

    inline void* Alloc(size_t size) {
        struct simple_allocator_list_entry *entry;
        if(header_->seg_off_ < header_->region_size) {
            entry = (struct simple_allocator_list_entry *)((char*)region_ + header_->seg_off_);
            header_->seg_off_ += header_->request_unit;
            return (void*)(entry + 1);
        }
        else if(header_->head_) {
            entry = (struct simple_allocator_list_entry *)((char*)region_ + header_->head_);
            header_->head_ = entry->next;
            return (void*)(entry + 1);
        }
        return nullptr;
    }

    inline void Free(void *data) {
        struct simple_allocator_list_entry *entry = (struct simple_allocator_list_entry *)data - 1;
        struct simple_allocator_list_entry *tail = (struct simple_allocator_list_entry *)((char*)region_ + header_->tail_);
        entry->next = 0;
        tail->next = (char*)entry - (char*)region_;
        header_->tail_ = tail->next;
        if(header_->head_ == 0) {
            header_->head_ = header_->tail_;
        }
    }
};

}

#endif

#endif //LABSTOR_SIMPLE_ALLOCATOR_H
