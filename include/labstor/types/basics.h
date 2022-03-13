//
// Created by lukemartinlogan on 11/30/21.
//

#ifndef LABSTOR_BASICS_H
#define LABSTOR_BASICS_H

#define MODULE_KEY_SIZE 32


#ifdef KERNEL_BUILD
#include <linux/types.h>
#elif __cplusplus
#include <stdint.h>
#include <string>
#include <cstring>
#include <unordered_map>
#endif

typedef uint32_t labstor_runtime_id_t;
typedef int32_t labstor_off_t;

struct labstor_id {
    char key_[MODULE_KEY_SIZE];
};

struct labstor_credentials {
    int pid_;
    int uid_;
    int gid_;
    int priority_;
};

#ifdef __cplusplus

namespace labstor {

struct id {
    char key_[MODULE_KEY_SIZE];
    id() = default;
    ~id() = default;
    id(std::string key_str) {
        strcpy(key_, key_str.c_str());
    }
    id(const char* key_str) {
        strcpy(key_, key_str);
    }
    bool operator==(const id &other) const {
        return strncmp(key_, other.key_, MODULE_KEY_SIZE) == 0;
    }
    void copy(const std::string &str) {
        memcpy(key_, str.c_str(), str.size());
        key_[str.size()] = 0;
    }
    const char& operator [](int i) {
        return key_[i];
    }
};

typedef int32_t off_t;
typedef labstor_credentials credentials;

}



namespace std {
    template<>
    struct hash<labstor::id> {
        std::size_t operator()(const labstor::id &id) const {
            size_t sum = 0;
            for (int i = 0; i < MODULE_KEY_SIZE; ++i) {
                if (id.key_[i] == 0) { break; }
                sum += id.key_[i] << (i % 8);
            }
            return sum;
        }
    };
}
#endif



#endif //LABSTOR_BASICS_H
