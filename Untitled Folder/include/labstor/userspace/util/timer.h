//
// Created by lukemartinlogan on 8/2/21.
//

#ifndef LABSTOR_TIMER_H
#define LABSTOR_TIMER_H

#include <chrono>

namespace labstor {

template<typename T>
class Timer {
private:
    std::chrono::time_point<T> start_, end_;
    double time_ns_;
public:
    Timer() : time_ns_(0) {}

    void Resume() {
        start_ = T::now();
    }

    double Pause() {
        end_ = T::now();
        time_ns_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();
        return time_ns_;
    }
    double GetMsecFromStart() {
        end_ = T::now();
        double elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();
        return elapsed/1000000;
    }

    double GetNsec() {
        return time_ns_;
    }

    double GetUsec() {
        return time_ns_/1000;
    }

    double GetMsec() {
        return time_ns_/1000000;
    }

    double GetSec() {
        return time_ns_/1000000000;
    }

    void Reset() {
        time_ns_ = 0;
    }

    double GetUsFromEpoch() {
        std::chrono::time_point<std::chrono::system_clock> point = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(point.time_since_epoch()).count();
    }
};

typedef Timer<std::chrono::high_resolution_clock> HighResCpuTimer;
typedef Timer<std::chrono::steady_clock> HighResMonotonicTimer;

}

#endif //LABSTOR_TIMER_Hv
