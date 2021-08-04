//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <sys/sysinfo.h>

void create_thread_pool(int nthreads) {
    for(int i = 0; i < nthreads; ++i) {

    }
}

int main(int argc, char **argv) {
    //Load a configuration file

    //Load the polling function

    create_thread_pool(get_nprocs());
}