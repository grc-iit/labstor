//
// Created by lukemartinlogan on 9/12/21.
//

#ifndef LABSTOR_BASICS_H
#define LABSTOR_BASICS_H

#ifdef __cplusplus

#include <string>
#include <cstring>
#include <unordered_map>

namespace labstor {

struct id {
    char key[64];
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
    void copy(const std::string &str) {
        memcpy(key, str.c_str(), str.size());
        key[str.size()] = 0;
    }
};

struct credentials {
    int pid;
    int uid;
    int gid;
    int priority;
};

}

template <>
struct std::hash<labstor::id> {
    std::size_t operator()(const labstor::id& id) const {
        return std::hash<char*>{}((char*)id.key);
    }
};

#endif

struct labstor_id {
    char key[64];
};

struct labstor_netlink_header {
    struct labstor_id module_id;
};

#endif //LABSTOR_BASICS_H
