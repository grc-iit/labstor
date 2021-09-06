//
// Created by lukemartinlogan on 8/21/21.
//

//[obj_alloc_header][data_struct_header]

#ifndef LABSTOR_OBJ_ALLOCATOR_LOCKLESS_H
#define LABSTOR_OBJ_ALLOCATOR_LOCKLESS_H

#include <labstor/shared_memory/shared_memory.h>
#include <limits>
#include <atomic>

namespace labstor::ipc::alloc {


inline phys_ptr seg_alloc(phys_ptr &segment_off, size_t size, size_t &segment_size) {
    phys_ptr old_segment = segment_off;
    segment_size -= size;
    segment_off += size;
    return old_segment;
}

struct free_list_entry {
    phys_ptr prior;
    inline static size_t size(size_t size) {
        return sizeof(free_list_entry) + size;
    }
};

struct free_list {
    std::atomic<phys_ptr> tail_;

    void append(shmem_ptr<free_list_entry> entry) {
        phys_ptr expected;
        do {
            expected = tail_.load(std::memory_order_relaxed);
            entry->prior = expected;
        } while(!std::atomic_compare_exchange_weak(&tail_, &entry->prior, entry.get_off()));
    }

    shmem_ptr<free_list_entry> pop(void *region) {
        shmem_ptr<free_list_entry> expected;
        expected.region_ = static_cast<char*>(region);
        do {
            expected.off_ = tail_.load(std::memory_order_relaxed);
            if(expected.IsNull()) { return expected; }
        } while(!std::atomic_compare_exchange_weak(&tail_, &expected.off_, expected->prior));
        return expected;
    }
};

struct obj_allocator {
    struct free_list free_list_;

    void init(void *region, phys_ptr segment, size_t segment_size, size_t obj_size) {
        size_t alloced_size = 0;
        free_list_.tail_ = -1;
        while(alloced_size < segment_size) {
            shmem_ptr<free_list_entry> page(region, segment + alloced_size);
            free_list_.append(page);
            alloced_size += sizeof(free_list_entry) + obj_size;
        }
    }

    void* alloc(void *region, size_t size) {
        shmem_ptr<free_list_entry> entry;
        entry = free_list_.pop(region);
        if(entry.IsNull()) { return nullptr; }
        return static_cast<void*>(entry.get() + 1);
    }

    void free(void *region, void *ptr) {
        struct free_list_entry *entry_proc = static_cast<struct free_list_entry*>(ptr) - 1;
        shmem_ptr<free_list_entry> entry(region, entry_proc);
        free_list_.append(entry);
    }
};

template<typename H>
struct hashed_obj_allocator_header {
    H header_;
    size_t num_buckets_;
    phys_ptr buckets_;

    struct obj_allocator* init(void *region, size_t &region_off, size_t &region_size, size_t num_buckets) {
        size_t buckets_size = num_buckets_ * sizeof(struct obj_allocator);
        buckets_ = seg_alloc(region_off, buckets_size, region_size);
        num_buckets_ = num_buckets;
        shmem_ptr<struct obj_allocator> buckets(region, buckets_);
        memset((void*)buckets.get(), 0, buckets_size);
        return buckets.get();
    }

    struct obj_allocator* get_buckets(void *region) {
        return shmem_ptr<struct obj_allocator>(region, buckets_).get();
    }

    inline size_t map_id(size_t id) {
        return id%num_buckets_;
    }
};

template<typename H>
class hashed_obj_allocator_lockless {
private:
    void *region_;
    struct hashed_obj_allocator_header<H> *allocator_;

    size_t num_allocators_;
    struct obj_allocator* obj_allocators_;
    H *header_;
    size_t id_;

public:
    void init(size_t id, void *region, size_t region_size, size_t num_buckets, size_t obj_size) {
        size_t region_off = 0;

        id_ = id;
        region_ = region;
        allocator_ = shmem_ptr<hashed_obj_allocator_header<H>>(region, seg_alloc(region_off, sizeof(hashed_obj_allocator_header<H>), region_size)).get();
        num_allocators_ = num_buckets;
        obj_allocators_ = allocator_->init(region, region_off, region_size, num_buckets);
        header_ = &allocator_->header_;

        size_t segment_size = region_size / num_buckets;
        for(int i = 0; i < num_buckets; ++i) {
            struct obj_allocator *obj_alloc = obj_allocators_ + i;
            phys_ptr segment = seg_alloc(region_off, segment_size, region_size);
            obj_alloc->init(region, segment, segment_size, obj_size);
        }
    }

    void open(size_t id, void *region) {
        id_ = id;
        region_ = region;
        allocator_ = static_cast<struct hashed_obj_allocator_header<H> *>(region);
        num_allocators_ = allocator_->num_buckets_;
        obj_allocators_ = allocator_->get_buckets(region);
        header_ = &allocator_->header_;
    }

    //Stores where the main data structure is
    H *header() {
        return header_;
    }

    void *alloc(size_t size) {
        void *ptr;
        struct obj_allocator *obj_alloc;
        size_t allocators_checked = num_allocators_;
        size_t obj_alloc_idx = allocator_->map_id(id_);
        while(allocators_checked > 0) {
            obj_alloc = obj_allocators_ + obj_alloc_idx;
            ptr = obj_alloc->alloc(region_, size);
            if(ptr) { return ptr; }
            else { --allocators_checked; }
            obj_alloc_idx = (obj_alloc_idx + 1) % num_allocators_;
        }
        return nullptr;
    }

    void free(void *ptr) {
        struct obj_allocator *obj_alloc;
        size_t obj_alloc_idx = allocator_->map_id(id_);
        obj_alloc = obj_allocators_ + obj_alloc_idx;
        obj_alloc->free(region_, ptr);
    }
};

}

#endif //LABSTOR_OBJ_ALLOCATOR_LOCKLESS_H
