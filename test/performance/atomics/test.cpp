//
// Created by lukemartinlogan on 12/12/21.
//

#include <stdio.h>
#include <labstor/userspace/util/timer.h>

const int hi = 124;

void test_atomics(int total_reqs) {
    labstor::HighResMonotonicTimer t[4];
    uint32_t number = 0;

    t[0].Resume();
    for(int i = 0; i < total_reqs; ++i) {
        __atomic_compare_exchange_n(&number, &number, hi+1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    }
    t[0].Pause();

    t[1].Resume();
    for(int i = 0; i < total_reqs; ++i) {
        __atomic_fetch_add(&number, 1, __ATOMIC_RELAXED);
    }
    t[1].Pause();

    t[2].Resume();
    for(int i = 0; i < total_reqs; ++i) {
        __atomic_fetch_and(&number, 1, __ATOMIC_RELAXED);
    }
    t[2].Pause();

    t[3].Resume();
    for(int i = 0; i < total_reqs; ++i) {
        __atomic_fetch_or(&number, 1, __ATOMIC_RELAXED);
    }
    t[3].Pause();

    printf("cmpx=%lf, add=%lf, and=%lf, or=%lf\n",
           total_reqs/t[0].GetMsec(),
           total_reqs/t[1].GetMsec(),
           total_reqs/t[2].GetMsec(),
           total_reqs/t[3].GetMsec());
}

int main() {
    test_atomics(10000000);
}