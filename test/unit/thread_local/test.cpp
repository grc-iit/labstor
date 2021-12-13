//
// Created by lukemartinlogan on 12/12/21.
//

#include <labstor/types/thread_local.h>
#include <omp.h>

namespace labstor {
    uint32_t thread_local_counter_ = 0;
    thread_local uint32_t thread_local_tid_;
    thread_local uint32_t thread_local_initialized_;
}

int main() {
    int nthreads = 20;
#pragma omp parallel num_threads(nthreads)
    {
        printf("ThreadID: %d\n", labstor::ThreadLocal::GetTid());
#pragma omp barrier
    }
}