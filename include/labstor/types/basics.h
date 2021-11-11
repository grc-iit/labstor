//
// Created by lukemartinlogan on 9/12/21.
//

#ifndef LABSTOR_BASICS_H
#define LABSTOR_BASICS_H

struct labstor_id {
    char key[256];
};

struct labstor_netlink_header {
    struct labstor_id module_id;
};

#ifdef __cplusplus

#include <cstring>
#include <unordered_map>

namespace labstor {

struct id {
    char key[256];
    id() = default;
    ~id() = default;
    id(std::string key_str) {
        strcpy(key, key_str.c_str());
    }
    id(const char* key_str) {
        strcpy(key, key_str);
    }
    bool operator==(const id &other) const {
        return strcmp(key, other.key);
    }
};

struct credentials {
    int pid;
    int uid;
    int gid;
    int priority;
};

struct setup_request {
    int num_queues;
};

struct setup_reply {
    int region_id;
    size_t region_size;
};

struct SpinLock {
    int lock_;

    SpinLock() { Init(); }
    inline void Init() {
        lock_ = 0;
    }
    inline void Lock() {
        int unlocked = 0;
        do {} while (__atomic_compare_exchange_n(&lock_, &unlocked, 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    }
    inline void UnLock() {
        lock_ = 0;
    }
    inline int TryLock() {
        int unlocked = 0;
        return __atomic_compare_exchange_n(&lock_, &unlocked, 1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    }
};

}

template <>
struct std::hash<labstor::id> {
    std::size_t operator()(const labstor::id& id) const {
        return std::hash<char*>{}((char*)id.key);
    }
};

#endif

#endif //LABSTOR_BASICS_H
