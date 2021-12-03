//
// Created by lukemartinlogan on 11/27/21.
//

#ifndef LABSTOR_DEBUG_H
#define LABSTOR_DEBUG_H

#include "stdio.h"
#include "timer.h"
#include "serializeable.h"

#define LABSTOR_DEBUG
#ifdef LABSTOR_DEBUG
#define AUTO_TRACE(...) labstor::AutoTrace auto_tracer(false, __VA_ARGS__);
#define TRACEPOINT(...) labstor::AutoTrace(true, __VA_ARGS__);
#else
#define AUTO_TRACE(...)
#endif

namespace labstor {

class AutoTrace {
private:
    std::string base_text_;
    bool tracepoint_;
public:
    template<typename ...Args>
    AutoTrace(bool tracepoint, Args ...args) : tracepoint_(tracepoint) {
        labstor::ArgPacker params(args...);
        char *buffer = (char *) calloc(8192, 1);
        size_t off = 0;
        int arg = 0;
        for (auto &param : params) {
            off += params[arg++].serialize(buffer + off);
            buffer[off++] = ';';
        }
        base_text_ = std::string(buffer, off);
        if(!tracepoint) { printf("%s\n", (base_text_ + "start").c_str()); }
        else { printf("%s\n", (base_text_).c_str()); }
    }

    ~AutoTrace() {
        if(!tracepoint_) {
            printf("%s\n", (base_text_ + "end").c_str());
        }
    }
};

}

#endif //LABSTOR_DEBUG_H
