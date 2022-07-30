
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LABSTOR_STRING_H
#define LABSTOR_STRING_H

#ifdef __cplusplus

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
    uint32_t length_;

    inline void *GetRegion() {
        return (void*)header_;
    }

    inline string() : header_(nullptr), data_(nullptr), length_(0) {}

    inline string(const std::string &str, labstor::GenericAllocator *alloc) {
        header_ = (string_header*)alloc->Alloc(str.size());
        header_->length_ = str.size();
        length_ = header_->length_;
        data_ = (char*)(header_ + 1);
        memcpy(data_, str.c_str(), str.size());
        data_[length_] = 0;
    }

    inline string(char *str) {
        data_ = str;
        length_ = strlen(str);
    }

    inline string(char *str, int length) {
        data_ = str;
        length_ = length;
    }
    inline string(labstor::id &id) {
        data_ = id.key_;
        length_ = strnlen(id.key_, MODULE_KEY_SIZE);
    }
    inline string(const std::string &str) {
        data_ = reinterpret_cast<char*>(malloc(str.size()));
        strncpy(data_, str.c_str(), str.size());
        length_ = str.size();
    }
    inline string(const string &old_str) {
        header_ = old_str.header_;
        data_ = old_str.data_;
        length_ = old_str.length_;
    }

    inline void Init(void *region, std::string str) {
        header_ = (string_header*)region;
        data_ = (char*)(header_ + 1);
        memcpy(data_, str.c_str(), str.size());
        header_->length_ = str.size();
        length_ = header_->length_;
        data_[header_->length_] = 0;
    }

    inline void Attach(void *region) {
        header_ = (string_header*)region;
        data_ = (char*)(header_ + 1);
        length_ = header_->length_;
    }
    std::string ToString() {
        return std::string(c_str(), size());
    }
    inline uint32_t Hash() {
        return hash(c_str(), size());
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
    inline bool operator ==(std::nullptr_t null) const {
        return length_ == 0;
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
    inline uint32_t size() const { return length_; }

    static inline uint32_t hash(const char *key, const uint32_t length) {
        uint32_t sum = 0;
        for(size_t i = 0; i < length; ++i) {
            sum += key[i] << 4*(i%4);
        }
        return sum;
    }
};


}

#endif

#endif //LABSTOR_STRING_H