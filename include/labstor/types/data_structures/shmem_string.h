//
// Created by lukemartinlogan on 11/21/21.
//

#ifndef LABSTOR_STRING_H
#define LABSTOR_STRING_H

#include <string>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/allocator.h>

namespace labstor::ipc {

struct string_header {
    uint32_t length_;
};

struct string {
    string_header *header_;
    char *data_;

    string() : header_(nullptr), data_(nullptr) {}

    inline void Init(void *region_, std::string str) {
        header_ = (string_header*)region_;
        data_ = (char*)(header_ + 1);
        memcpy(data_, str.c_str(), str.size());
        header_->length_ = str.size();
        data_[header_->length_] = 0;
    }

    inline void Attach(void *region_) {
        header_ = (string_header*)region_;
        data_ = (char*)(header_ + 1);
    }

    inline char& operator [](int idx) const {
        return data_[idx];
    }
    inline bool operator ==(labstor::ipc::string str) const {
        if(str.size() != size()) { return false; }
        return memcmp(data_, str.c_str(), size()) == 0;
    }
    inline bool operator ==(const std::string &str) const {
        if(str.size() != size()) { return false; }
        return memcmp(data_, str.c_str(), size()) == 0;
    }
    inline bool operator !=(labstor::ipc::string str) const {
        if(str.size() != size()) { return false; }
        return memcmp(data_, str.c_str(), size()) != 0;
    }
    inline bool operator !=(const std::string &str) const {
        if(str.size() != size()) { return false; }
        return memcmp(data_, str.c_str(), size()) != 0;
    }
    inline char* c_str() const {
        return data_;
    }
    inline uint32_t size() const { return header_ ? header_->length_ : 0; }

    static inline uint32_t hash(const char *key, const uint32_t length) {
        uint32_t sum = 0;
        for(int i = 0; i < length; ++i) {
            sum += key[i] << 4*(i%4);
        }
        return sum;
    }
};


}

#endif //LABSTOR_STRING_H