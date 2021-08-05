//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <sys/sysinfo.h>

#include <labstor/interface/interface.h>

void create_thread_pool(int nthreads, poll_requests_fn poll) {
    pid_t child, parent;
    void *stack;
    for(int i = 0; i < nthreads; ++i) {
        clone(poll, void *stack, int flags, void *arg, &parent, &child);
    }
}

int main(int argc, char **argv) {
//Load a configuration file

//Load the polling function

create_thread_pool(get_nprocs());
}