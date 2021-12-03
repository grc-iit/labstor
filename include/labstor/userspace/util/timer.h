//
// Created by lukemartinlogan on 8/2/21.
//

#ifndef LABSTOR_TIMER_H
#define LABSTOR_TIMER_H

#include <chrono>

namespace labstor {

class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_, end_;
    double time_ns_;
public:
    Timer() : time_ns_(0) {}

    void Resume() {
        start_ = std::chrono::high_resolution_clock::now();
    }

    double Pause() {
        end_ = std::chrono::high_resolution_clock::now();
        time_ns_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();
        return time_ns_;
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
};

}

#endif //LABSTOR_TIMER_Hv
