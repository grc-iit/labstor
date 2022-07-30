
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

#ifndef LABSTOR_ERROR_SERIALIZER_H
#define LABSTOR_ERROR_SERIALIZER_H

#include <memory>
#include <vector>
#include <string>
#include <type_traits>
#include <cstring>

#define NUMBER_SERIAL(type) \
    std::string new_str = std::to_string(num_.type); \
    memcpy(buf, new_str.c_str(), new_str.size()); \
    return new_str.size();

namespace labstor {

    class Serializeable {
    public:
        virtual size_t serialize(char *buf) = 0;
    };

    class SizeType : public Serializeable {
    public:
        double num_;
        size_t unit_;

        static const size_t
                BYTES=1,
                KB=(1ul << 10),
                MB=(1ul << 20),
                GB=(1ul << 30),
                TB=(1ul << 40);

        std::string unit_to_str(size_t unit) {
            switch(unit) {
                case BYTES: return "BYTES";
                case KB: return "KB";
                case MB: return "MB";
                case GB: return "GB";
                case TB: return "TB";
            }
            return "";
        }

        SizeType() : num_(0), unit_(0) {}
        SizeType(const SizeType &old_obj) {
            num_ = old_obj.num_;
            unit_ = old_obj.unit_;
        }

        SizeType(int8_t bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(int16_t bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(int32_t bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(int64_t bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(uint8_t bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(uint16_t bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(uint32_t bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(uint64_t bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(float bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}
        SizeType(double bytes, size_t unit) : num_(((double)bytes)/unit), unit_(unit) {}

        size_t serialize(char *buf) {
            std::string serial = ToString();
            memcpy(buf, serial.c_str(), serial.size());
            printf("serial: %s\n", serial.c_str());
            return serial.size();
        }

        std::string ToString() {
            return std::to_string(num_) + unit_to_str(unit_);
        }
    };

    class Arg : Serializeable {
    private:
        union {
            char d8;
            short int d16;
            int d32;
            long d64;
            long long d128;
            unsigned char u8;
            unsigned short int u16;
            unsigned u32;
            unsigned long u64;
            unsigned long long u128;
            float f32;
            double f64;
            long double f96;
        } num_;
        std::string str_;
        Serializeable *obj_;
        std::shared_ptr<Serializeable> obj_shared_;
        int type_;
    public:
        /*
        Arg(char num) : type_(0) { num_.d8 = num; }
        Arg(short int num) : type_(0) { num_.d16 = num; }
        Arg(int num) : type_(0) { num_.d32 = num; }
        Arg(long num) : type_(1) { num_.d64 = num; }
        Arg(long long num) : type_(2) { num_.d128 = num; }
        Arg(unsigned char num) : type_(3) { num_.u8 = num; }
        Arg(unsigned short int num) : type_(3) { num_.u16 = num; }
        Arg(unsigned int num) : type_(3) { num_.u32 = num; }
        Arg(unsigned long num) : type_(4) { num_.u64 = num; }
        Arg(unsigned long long num) : type_(5) { num_.u128 = num; }*/
        Arg(int8_t num) : type_(0) { num_.d8 = num; }
        Arg(int16_t num) : type_(1) { num_.d16 = num; }
        Arg(int32_t num) : type_(2) { num_.d32 = num; }
        Arg(int64_t num) : type_(3) { num_.d64 = num; }
        Arg(uint8_t num) : type_(4) { num_.u8 = num; }
        Arg(uint16_t num) : type_(5) { num_.u16 = num; }
        Arg(uint32_t num) : type_(6) { num_.u32 = num; }
        Arg(uint64_t num) : type_(7) { num_.u64 = num; }
        Arg(float num) : type_(8) { num_.f32 = num; }
        Arg(double num) : type_(9) { num_.f64 = num; }
        Arg(long double num) : type_(10) { num_.f96 = num; }
        Arg(char *str) : type_(11) { if(str) str_ = std::string(str); else str_ = ""; }
        Arg(const char *str) : type_(11) { if(str) str_ = std::string(str); else str_ = ""; }
        Arg(std::string str) : type_(11) { str_ = str; }
        Arg(Serializeable *obj) : type_(12) { obj_ = obj; }
        Arg(std::shared_ptr<Serializeable> obj) : type_(13) { obj_shared_ = obj; }
        template<typename T>
        Arg(T obj) : type_(11) { obj_shared_ = std::shared_ptr<Serializeable>(new T(obj)); }

        size_t serialize(char *buf) {
            switch(type_) {
                case 0: {
                    NUMBER_SERIAL(d8)
                }
                case 1: {
                    NUMBER_SERIAL(d16)
                }
                case 2: {
                    NUMBER_SERIAL(d32)
                }
                case 3: {
                    NUMBER_SERIAL(d64)
                }
                case 4: {
                    NUMBER_SERIAL(u8)
                }
                case 5: {
                    NUMBER_SERIAL(u16)
                }
                case 6: {
                    NUMBER_SERIAL(u32)
                }
                case 7: {
                    NUMBER_SERIAL(u64)
                }
                case 8: {
                    NUMBER_SERIAL(f32)
                }
                case 9: {
                    NUMBER_SERIAL(f64)
                }
                case 10: {
                    NUMBER_SERIAL(f96)
                }
                case 11: {
                    memcpy(buf, str_.c_str(), str_.size());
                    return str_.size();
                }
                case 12: {
                    return obj_->serialize(buf);
                }
                case 13: {
                    return obj_shared_->serialize(buf);
                }
            }
            return 0;
        }
    };

    class ArgPacker {
    private:
        std::vector<Arg> args_;
    public:
        template<typename ...Args>
        ArgPacker(Args ...args) {
            args_ = {args...};
        }
        Arg& operator [](int pos) { return args_[pos]; }

        std::vector<Arg>::iterator begin() { return args_.begin(); }
        std::vector<Arg>::iterator end() { return args_.end(); }
        std::vector<Arg>::reverse_iterator rbegin() { return args_.rbegin(); }
        std::vector<Arg>::reverse_iterator rend() { return args_.rend(); }
    };

    class Formatter {
    public:
        template<typename ...Args>
        static std::string format(std::string fmt, Args ...args) {
            ArgPacker params(args...);
            char *buffer = (char*)calloc(8192, 1);
            size_t off = 0;
            int arg = 0;
            for (size_t i = 0; i < fmt.size(); ++i) {
                if (fmt[i] == '{' && fmt[i + 1] == '}') {
                    off += params[arg++].serialize(buffer + off);
                    ++i;
                    continue;
                }
                buffer[off++] = fmt[i];
            }
            std::string str(buffer, off);
            free(buffer);
            return str;
        }
    };
}

#endif //LABSTOR_ERROR_SERIALIZER_H