//
// Created by lukemartinlogan on 11/27/21.
//

#ifndef LABSTOR_DEBUG_H
#define LABSTOR_DEBUG_H

#if defined(LABSTOR_DEBUG) && defined(__cplusplus)
#define AUTO_TRACE(...) labstor::AutoTrace auto_tracer(false, __PRETTY_FUNCTION__, __VA_ARGS__);
#define TRACEPOINT(...) labstor::AutoTrace(true, __PRETTY_FUNCTION__, __VA_ARGS__);
#elif defined(KERNEL_BUILD) && defined(DEBUG)
#define AUTO_TRACE(...) pr_info(__VA_ARGS__);
#define TRACEPOINT(...) pr_info(__VA_ARGS__);
#else
#define AUTO_TRACE(...)
#define TRACEPOINT(...)
#endif

#ifdef __cplusplus

#include "stdio.h"
#include "labstor/userspace/util/timer.h"
#include "labstor/userspace/util/serializeable.h"
#include <sched.h>
#include <unistd.h>

namespace labstor {

class AutoTrace {
private:
    std::string base_text_;
    bool tracepoint_;
    labstor::HighResCpuTimer t_cpu_;
    labstor::HighResMonotonicTimer t_total_;
public:
    template<typename ...Args>
    AutoTrace(bool tracepoint, Args ...args) : tracepoint_(tracepoint) {
        labstor::ArgPacker params(args...);
        char *buffer = (char *) calloc(8192, 1);
        size_t off = 0;
        for (auto &param : params) {
            off += param.serialize(buffer + off);
            buffer[off++] = ';';
        }
        base_text_ = std::string(buffer, off) + ";";
        t_cpu_.Resume();
        t_total_.Resume();
        if(!tracepoint) { printf("%s\n", (base_text_ + "start").c_str()); }
        else { printf("%d;%s\n", gettid(),(base_text_).c_str()); }
    }

    ~AutoTrace() {
        if(!tracepoint_) {
            t_cpu_.Pause();
            t_total_.Pause();
            base_text_ += "cpu-time-us;" + std::to_string(t_cpu_.GetUsec()) + ";total-time-us;" + std::to_string(t_total_.GetUsec()) + ";";
            printf("%s\n", (base_text_ + "end").c_str());
        }
    }
};

}

#endif

#endif //LABSTOR_DEBUG_H
