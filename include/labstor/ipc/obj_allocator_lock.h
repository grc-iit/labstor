//
// Created by lukemartinlogan on 8/21/21.
//

//[obj_alloc_header][data_struct_header]

#ifndef LABSTOR_OBJ_ALLOCATOR_LOCK_H
#define LABSTOR_OBJ_ALLOCATOR_LOCK_H

#include <labstor/shared_memory/shared_memory.h>
#include <limits>
#include <functional>

namespace labstor::ipc::alloc {

struct free_list_entry {
    struct free_list_entry *next;
    inline static size_t size(size_t size) {
        return sizeof(free_list_entry) + size;
    }
};

struct free_list {
    struct free_list_entry *head_, *tail_;

    void append(struct free_list_entry *entry) {
        entry->next = nullptr;
        if(head_ == nullptr) {
            head_ = tail_ = entry;
            return;
        }
        tail_->next = entry;
        tail_ = tail_->next;
    }

    struct free_list_entry* pop() {
        struct free_list_entry *old_head = head_;
        if(head_ == tail_) {
            head_ = tail_ = nullptr;
            return old_head;
        }
        head_ = head_->next;
        return old_head;
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

inline void *seg_alloc(void *&region, size_t size, size_t &region_size) {
    void *old_region = region;
    region_size -= size;
    region = static_cast<void*>(static_cast<char*>(region) + size);
    return old_region;
}

struct obj_allocator {
    size_t segment_size_;
    void *segment_;
    SharedMemoryMutex lock_;
    struct free_list free_list_;

    inline int trylock() {
        return lock_.trylock();
    }

    void unlock() {
        lock_.unlock();
    }

    void* alloc(size_t size) {
        struct free_list_entry *entry;
        //Allocate an object from the segment
        if(segment_size_ >= free_list_entry::size(size)) {
            entry = static_cast<struct free_list_entry*>(seg_alloc(segment_, free_list_entry::size(size), segment_size_));
            return static_cast<void*>(entry + 1);
        }
        //Allocate an object from the free list
        entry = free_list_.pop();
        if(entry) {
            return static_cast<void*>(entry + 1);
        }
        return nullptr;
    }

    void free(void *ptr) {
        struct free_list_entry *entry = static_cast<struct free_list_entry*>(ptr) - 1;
        free_list_.append(entry);
    }
};

template<typename H>
struct hashed_obj_allocator_header {
    struct hashtable<size_t, struct obj_allocator> *obj_allocator_table_;
    H header_;

    inline size_t num_buckets() {
        return obj_allocator_table_->num_buckets_;
    }

    inline size_t map_id(size_t id) {
        return obj_allocator_table_->get_idx(id);
    }

    inline struct obj_allocator *get_allocator(int idx) {
        return obj_allocator_table_->arr_idx(idx);
    }
};

template<typename H>
class hashed_obj_allocator_lock {
private:
    struct hashed_obj_allocator_header<H> *allocator_;
    H *header_;
    size_t id_;

public:
    void init(size_t id, void *region, size_t region_size, size_t num_buckets, size_t obj_size) {
        id_ = id;
        allocator_ = static_cast<struct hashed_obj_allocator_header<H> *>(seg_alloc(region, sizeof(struct hashed_obj_allocator_header<H>), region_size));
        header_ = &allocator_->header_;
        allocator_->obj_allocator_table_ = hashtable_alloc<size_t, struct obj_allocator>(region, num_buckets, region_size);
        size_t segment_size = region_size / num_buckets;
        for(int i = 0; i < num_buckets; ++i) {
            struct obj_allocator *obj_alloc = allocator_->get_allocator(i);
            obj_alloc->segment_ = seg_alloc(region, segment_size, region_size);
            obj_alloc->segment_size_ = segment_size;
            obj_alloc->lock_.init();
        }
    }

    void open(size_t id, void *region) {
        id_ = id;
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
            if(obj_alloc->trylock()) {
                ptr = obj_alloc->alloc(size);
                if(ptr) {
                    obj_alloc->unlock();
                    return ptr;
                } else {
                    obj_alloc->unlock();
                    --allocators_checked;
                }
            }
            obj_alloc_idx = (obj_alloc_idx + 1) % allocator_->num_buckets();
        }
        return nullptr;
    }

    void free(void *ptr) {
        struct obj_allocator *obj_alloc;
        size_t allocators_checked_ = allocator_->num_buckets();
        size_t obj_alloc_idx = allocator_->map_id(id_);
        while(allocators_checked_ > 0) {
            obj_alloc = allocator_->get_allocator(obj_alloc_idx);
            if(obj_alloc->trylock()) {
                obj_alloc->free(ptr);
                obj_alloc->unlock();
                return;
            }
            obj_alloc_idx = (obj_alloc_idx + 1) % allocator_->num_buckets();
        }
    }

private:
    template<typename S, typename T>
    inline hashtable<S,T> *hashtable_alloc(void *&region, size_t num_buckets, size_t &region_size) {
        hashtable<S,T> *table = static_cast<hashtable<S,T>*>(seg_alloc(region, hashtable<S,T>::size(num_buckets), region_size));
        table->init(static_cast<void*>(table + 1), num_buckets);
        return table;
    }
};

}

#endif //LABSTOR_OBJ_ALLOCATOR_LOCK_H
