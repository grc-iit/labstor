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

template<typename T>
struct shmem_ptr {
    char *region_;
    size_t off_;

    shmem_ptr() = default;
    shmem_ptr(void *region, T *data) { init(region, data); }
    shmem_ptr(void *region, size_t  off) { init(region, off); }

    void init(void *region, T *data) {
        region_ = static_cast<char*>(region);
        off_ = (char*)(data) - (char*)region;
    }
    void init(void *region, size_t  off) {
        region_ = (char*)(region);
        off_ = off;
    }

    T* operator->() { return get(); }
    T* get() {
        if(IsNull()) { return nullptr; }
        return (T*)(region_ + off_ - 1);
    }
    inline bool IsNull() {
        return off_ == 0;
    }

    size_t get_off() {
        return off_;
    }
};

struct free_list_entry {
    size_t prior;
    inline static size_t size(size_t size) {
        return sizeof(free_list_entry) + size;
    }
};

struct free_list {
    std::atomic<size_t> tail_;

    void append(shmem_ptr<free_list_entry> entry) {
        size_t expected;
        do {
            expected = tail_.load(std::memory_order_relaxed);
            entry->prior = expected;
        } while(std::atomic_compare_exchange_weak(&tail_, &entry->prior, entry.get_off()));
    }

    shmem_ptr<free_list_entry> pop(void *region) {
        shmem_ptr<free_list_entry> expected;
        expected.region_ = static_cast<char*>(region);
        do {
            expected.off_ = tail_.load(std::memory_order_relaxed);
            if(expected.IsNull()) { return expected; }
        } while(std::atomic_compare_exchange_weak(&tail_, &expected.off_, expected->prior));
        return expected;
    }
};

template<typename S, typename T>
struct bucket {
    S key;
    T value;
};

template<typename S, typename T>
struct hashtable {
    size_t num_buckets_;
    bucket<S,T> *buckets_;

    inline static size_t size(size_t num_buckets) {
        return sizeof(hashtable<S,T>) + num_buckets*sizeof(bucket<S,T>);
    }

    void init(void *buckets, size_t num_buckets) {
        size_t buckets_size = num_buckets_ * sizeof(bucket<S,T>);
        buckets_ = static_cast<bucket<S,T>*>(buckets);
        num_buckets_ = num_buckets;
        memset((void*)buckets_, 0, buckets_size);
    }

    inline void create(S key, T val) {
        size_t bucket = get_idx(key);
        for(size_t i = 0; i < num_buckets_; ++i) {
            if(buckets_[bucket].key == 0) {
                buckets_[bucket].key = key + 1;
                buckets_[bucket].value = val;
                return;
            }
            bucket = (bucket + 1)%num_buckets_;
        }
    }

    inline void create(S key) {
        size_t bucket = get_idx(key);
        for(size_t i = 0; i < num_buckets_; ++i) {
            if(buckets_[bucket].key == 0) {
                buckets_[bucket].key = key + 1;
                return;
            }
            bucket = (bucket + 1)%num_buckets_;
        }
    }

    inline size_t get_idx(S key) {
        return key%num_buckets_;
    }

    inline T* arr_idx(int i) {
        return &buckets_[i].value;
    }
};

inline size_t seg_alloc(size_t &segment_off, size_t size, size_t &segment_size) {
    size_t old_segment = segment_off;
    segment_size -= size;
    segment_off += size;
    return old_segment;
}

struct obj_allocator {
    size_t segment_size_;
    size_t segment_off_;
    struct free_list free_list_;

    void* alloc(void *region, size_t size) {
        shmem_ptr<free_list_entry> entry;
        //Allocate an object from the segment
        if(segment_size_ >= free_list_entry::size(size)) {
            entry = shmem_ptr<free_list_entry>(region, seg_alloc(segment_off_, free_list_entry::size(size), segment_size_);
            return static_cast<void*>(entry.get() + 1);
        }
        //Allocate an object from the free list
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
    hashtable<size_t, struct obj_allocator> obj_allocator_table_;

    inline size_t num_buckets() {
        return obj_allocator_table_.num_buckets_;
    }

    inline size_t map_id(size_t id) {
        return obj_allocator_table_.get_idx(id);
    }

    inline struct obj_allocator *get_allocator(int idx) {
        return obj_allocator_table_.arr_idx(idx);
    }
};

template<typename H>
class hashed_obj_allocator_lockless {
private:
    void *region_;
    struct hashed_obj_allocator_header<H> *allocator_;
    struct hashtable<size_t, struct obj_allocator> *obj_allocator_table_;
    H *header_;
    size_t id_;

public:
    void init(size_t id, void *region, size_t region_size, size_t num_buckets, size_t obj_size) {
        id_ = id;
        region_ = region;
        size_t region_off_ = 0;
        allocator_ = shmem_ptr<hashed_obj_allocator_header<H>>(region, seg_alloc(region_off_, sizeof(struct hashed_obj_allocator_header<H>), region_size)).get();
        header_ = &allocator_->header_;
        allocator_->obj_allocator_table_ = hashtable_alloc<size_t, struct obj_allocator>(region, num_buckets, region_size);
        size_t segment_size = region_size / num_buckets;
        for(int i = 0; i < num_buckets; ++i) {
            struct obj_allocator *obj_alloc = allocator_->get_allocator(i);
            obj_alloc->segment_off_ = seg_alloc(region_off_, segment_size, region_size);
            obj_alloc->segment_size_ = segment_size;
        }
    }

    void open(size_t id, void *region) {
        id_ = id;
        region_ = region;
        allocator_ = static_cast<struct hashed_obj_allocator_header<H> *>(region);
        header_ = &allocator_->header_;
    }

    //Stores where the main data structure is
    H *header() {
        return header_;
    }

    void *alloc(size_t size) {
        void *ptr;
        struct obj_allocator *obj_alloc;
        size_t allocators_checked = allocator_->num_buckets();
        size_t obj_alloc_idx = allocator_->map_id(id_);
        while(allocators_checked > 0) {
            obj_alloc = allocator_->get_allocator(obj_alloc_idx);
            ptr = obj_alloc->alloc(size);
            if(ptr) { return ptr; }
            else { --allocators_checked; }
            obj_alloc_idx = (obj_alloc_idx + 1) % allocator_->num_buckets();
        }
        return nullptr;
    }

    void free(void *ptr) {
        struct obj_allocator *obj_alloc;
        size_t obj_alloc_idx = allocator_->map_id(id_);
        obj_alloc = allocator_->get_allocator(obj_alloc_idx);
        obj_alloc->free(ptr);
    }
};

}

#endif //LABSTOR_OBJ_ALLOCATOR_LOCKLESS_H
