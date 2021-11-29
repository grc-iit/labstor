//
// Created by lukemartinlogan on 9/12/21.
//

#ifndef LABSTOR_BASICS_H
#define LABSTOR_BASICS_H

#define MODULE_KEY_SIZE 64

#ifdef __cplusplus

#include <string>
#include <cstring>
#include <unordered_map>

namespace labstor {

struct id {
    char key[MODULE_KEY_SIZE];
    id() = default;
    ~id() = default;
    id(std::string key_str) {
        strcpy(key, key_str.c_str());
    }
    id(const char* key_str) {
        strcpy(key, key_str);
    }
    bool operator==(const id &other) const {
        return strncmp(key, other.key, MODULE_KEY_SIZE) == 0;
    }
    void copy(const std::string &str) {
        memcpy(key, str.c_str(), str.size());
        key[str.size()] = 0;
    }
    const char& operator [](int i) {
        return key[i];
    }
};

struct credentials {
    int pid;
    int uid;
    int gid;
    int priority;
};

typedef int32_t off_t;

}

namespace std {
    template<>
    struct hash<labstor::id> {
        std::size_t operator()(const labstor::id &id) const {
            size_t sum = 0;
            for (int i = 0; i < MODULE_KEY_SIZE; ++i) {
                if (id.key[i] == 0) { break; }
                sum += id.key[i] << (i % 8);
            }
            return sum;
        }
    };
}

#endif

struct labstor_id {
    char key[MODULE_KEY_SIZE];
};

struct labstor_netlink_header {
    struct labstor_id module_id;
};

#endif //LABSTOR_BASICS_H
