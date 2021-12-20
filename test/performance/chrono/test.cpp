//
// Created by lukemartinlogan on 12/20/21.
//

#include <stdio.h>
#include <labstor/userspace/util/timer.h>
#include <future>

int main() {
    labstor::HighResCpuTimer t[2], t_test;
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    for(int i = 0; i < 10000; ++i) {
        t[0].Resume();
        t_test.Resume();
        t[0].Pause();
    }

    for(int i = 0; i < 10000; ++i) {
        t[1].Resume();
        future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready;
        t[1].Pause();
    }

    printf("Thrpt: %lf Kops\n", 1000/t[0].GetMsec());
    printf("Thrpt: %lf Kops\n", 1000/t[1].GetMsec());
}