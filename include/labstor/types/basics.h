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
    id(std::string key_str) {
        strcpy(key, key_str.c_str());
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
    size_t queue_size;
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
