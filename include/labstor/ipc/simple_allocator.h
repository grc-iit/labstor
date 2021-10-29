//
// Created by lukemartinlogan on 9/19/21.
//

#ifndef LABSTOR_SIMPLE_ALLOCATOR_H
#define LABSTOR_SIMPLE_ALLOCATOR_H

#ifdef __cplusplus

#include <stddef.h>

namespace labstor {

struct labstor_simple_allocator_list_entry {
    size_t next;
};

struct labstor_simple_allocator_header {
    size_t region_size;
    size_t request_unit;
    size_t head_, tail_;
    size_t seg_off_;
};

struct labstor_simple_allocator {
    void *region_;
    struct labstor_simple_allocator_header *header_;
    char *alloc_data_;

    inline void Init(void *region, size_t region_size, size_t request_unit) {
        region_ = region;
        header_ = (struct labstor_simple_allocator_header*)region;
        header_->request_unit = request_unit;
        header_->region_size = region_size;
        header_->head_ = 0;
        header_->seg_off_ = 0;
        alloc_data_ = (char*)(header_ + 1);
    }

    inline void Attach(void *region, size_t region_size) {
        region_ = region;
        header_ = (struct labstor_simple_allocator_header*)region;
        alloc_data_ = (char*)(header_ + 1);
    }

    inline void* Alloc(size_t size) {
        struct labstor_simple_allocator_list_entry *entry;
        if(header_->seg_off_ < header_->region_size) {
            entry = (struct labstor_simple_allocator_list_entry *)(alloc_data_ + header_->seg_off_);
            header_->seg_off_ += header_->request_unit;
            return (void*)(entry + 1);
        }
        else if(header_->head_) {
            entry = (struct labstor_simple_allocator_list_entry *)(alloc_data_ + header_->head_);
            header_->head_ = entry->next;
            return (void*)(entry + 1);
        }
        return nullptr;
    }

    inline void Free(void *data) {
        struct labstor_simple_allocator_list_entry *entry = (struct labstor_simple_allocator_list_entry *)data - 1;
        struct labstor_simple_allocator_list_entry *tail = (struct labstor_simple_allocator_list_entry *)(alloc_data_ + header_->tail_);
        entry->next = 0;
        tail->next = (char*)entry - alloc_data_;
        header_->tail_ = tail->next;
        if(header_->head_ == 0) {
            header_->head_ = header_->tail_;
        }
    }
};

}

#endif

#ifdef KERNEL_BUILD
#include <linux/types.h>
#endif

struct labstor_simple_allocator_list_entry {
    size_t next;
};

struct labstor_simple_allocator_header {
    size_t region_size;
    size_t request_unit;
    size_t head_, tail_;
    size_t seg_off_;
};

struct labstor_simple_allocator {
    void *region_;
    struct labstor_simple_allocator_header *header_;
    char *alloc_data_;
};

#ifdef KERNEL_BUILD

static inline void labstor_allocator_init(struct labstor_simple_allocator *alloc, void *region, size_t region_size, size_t request_unit) {
    alloc->region_ = region;
    alloc->header_ = (struct labstor_simple_allocator_header*)region;
    alloc->header_->request_unit = request_unit;
    alloc->header_->region_size = region_size;
    alloc->header_->head_ = 0;
    alloc->header_->seg_off_ = 0;
    alloc->alloc_data_ = (char*)(alloc->header_ + 1);
}

static inline void labstor_allocator_attach(struct labstor_simple_allocator *alloc, void *region, size_t region_size) {
    alloc->region_ = region;
    alloc->header_ = (struct labstor_simple_allocator_header*)region;
    alloc->alloc_data_ = (char*)(alloc->header_ + 1);
}

static inline void* labstor_allocator_alloc(struct labstor_simple_allocator *alloc, size_t size) {
    struct labstor_simple_allocator_list_entry *entry;
    if(alloc->header_->seg_off_ < alloc->header_->region_size) {
        entry = (struct labstor_simple_allocator_list_entry *)(alloc->alloc_data_ + alloc->header_->seg_off_);
        alloc->header_->seg_off_ += alloc->header_->request_unit;
        return (void*)(entry + 1);
    }
    else if(alloc->header_->head_) {
        entry = (struct labstor_simple_allocator_list_entry *)(alloc->alloc_data_ + alloc->header_->head_);
        alloc->header_->head_ = entry->next;
        return (void*)(entry + 1);
    }
    return NULL;
}

static inline void labstor_allocator_free(struct labstor_simple_allocator *alloc, void *data) {
    struct labstor_simple_allocator_list_entry *entry = (struct labstor_simple_allocator_list_entry *)data - 1;
    struct labstor_simple_allocator_list_entry *tail = (struct labstor_simple_allocator_list_entry *)(alloc->alloc_data_ + alloc->header_->tail_);
    entry->next = 0;
    tail->next = (char*)entry - alloc->alloc_data_;
    alloc->header_->tail_ = tail->next;
    if(alloc->header_->head_ == 0) {
        alloc->header_->head_ = alloc->header_->tail_;
    }
}

#endif

#endif //LABSTOR_SIMPLE_ALLOCATOR_H
