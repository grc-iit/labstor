//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <sys/sysinfo.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <labstor/interface/interface.h>
#include <singleton.h>

std::unordered_map<pid_t,

void create_thread_pool(int nthreads, poll_requests_fn poll) {
    pid_t child, parent;
    void *stack;
    for(int i = 0; i < nthreads; ++i) {
        clone(poll, void *stack, int flags, void *arg, &parent, &child);
    }
}

void register_process(pid_t process) {
}

void poll_request_queues() {
}

int main(int argc, char **argv) {
    //Load a configuration file

    //Load the polling function

    //Create the shared memory space
    boost::interprocess::shared_memory_object queues {boost::interprocess::create, "LabstorServer", boost::interprocess::read_or_write};
    queues.truncate();

    create_thread_pool(get_nprocs());
}