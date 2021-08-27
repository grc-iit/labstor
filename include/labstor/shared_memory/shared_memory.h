//
// Created by lukemartinlogan on 8/24/21.
//

#ifndef LABSTOR_SHARED_MEMORY_H
#define LABSTOR_SHARED_MEMORY_H

#include <string>
#include <pthread.h>

namespace labstor::ipc {

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
