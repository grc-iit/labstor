//
// Created by lukemartinlogan on 8/24/21.
//

#ifndef LABSTOR_SHARED_MEMORY_H
#define LABSTOR_SHARED_MEMORY_H

#include <string>
#include <pthread.h>

namespace labstor::ipc {

typedef size_t phys_ptr;

template<typename T>
struct shmem_ptr {
    char *region_;
    phys_ptr off_;

    shmem_ptr() : region_(nullptr), off_(-1) {}
    shmem_ptr(void *region, T *data) { init(region, data); }
    shmem_ptr(void *region, phys_ptr  off) { init(region, off); }

    void init(void *region, T *data) {
        region_ = static_cast<char*>(region);
        off_ = (char*)(data) - (char*)region;
    }
    void init(void *region, phys_ptr off) {
        region_ = (char*)(region);
        off_ = off;
    }

    T* operator->() { return get(); }
    T& operator[](int idx) { return get()[idx]; }
    T* get() {
        if(IsNull()) { return nullptr; }
        return (T*)(region_ + off_);
    }
    inline bool IsNull() {
        return off_ == -1;
    }

    phys_ptr get_off() {
        return off_;
    }
};

struct SharedMemoryMutex {
    pthread_mutex_t mutex_;
    time_t timestamp_;

    SharedMemoryMutex() { init(); }
    void init() {
        pthread_mutexattr_t attr;
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mutex_, &attr);
    }
    void destroy() {
        pthread_mutex_destroy(&mutex_);
    }
    int lock() {
        return pthread_mutex_lock(&mutex_);
    }
    int trylock() {
        return pthread_mutex_trylock(&mutex_);
    }
    int unlock() {
        return pthread_mutex_unlock(&mutex_);
    }
};

struct SharedMemory {
    virtual void init(std::string path, size_t size) = 0;
    virtual void open_rw(std::string path) = 0;
    virtual void open_ro(std::string path) = 0;
    virtual void remove() = 0;
    virtual void remove(std::string path) = 0;
    virtual void *get_address() = 0;
};

}

#endif //LABSTOR_SHARED_MEMORY_H
