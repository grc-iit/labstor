//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/syscall.h>    /* Definition of SYS_* constants */
#include <unistd.h>
#include <pthread.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <labstor/interface/interface.h>
#include <labstor/util/singleton.h>
#include <ptrhead>

#include <unordered_map>
#include <list>

struct LabStorContext {
    labstor::ipc::unordered_map<std::string, struct io_system> namespace_;
    std::unorderd_map<pid_t process, std::list<struct queue_pair>> per_process_qps_;

    int num_io_workers_;
    std::vector<int> io_workers_;
    int sys_monitor_;
    int load_balancer_;
};

void poll_request_queues(void *data);

int create_thread(std::string error) {
    pid_t child_pid, parent_pid;
    if(clone(poll_request_queues, malloc(1<<20), CLONE_FILES | CLONE_FS | CLONE_VM, nullptr, &parent_pid, &child_pid) < 0) {
        std::cout << error << std::endl;
        exit(1);
    }
    return parent_pid;
}

void create_thread_pool(int nthreads) {
    pid_t child_pid, parent_pid;

    //Load balancer thread
    Singleton<struct LabStorContext>->load_balancer_ = create_thread("ERROR: could not create load balancer");

    //System monitor thread
    Singleton<struct LabStorContext>->sys_monitor_ = create_thread("ERROR: could not create system monitor");

    //I/O worker threads
    io_workers_.reserve(nthreads);
    for(int i = 0; i < nthreads; ++i) {
        Singleton<struct LabStorContext>->io_workers_[i] = create_thread("ERROR: could not create I/O worker");
    }
}

void allocate_queues(pid_t process) {
}

void poll_request_queues(void *nothing) {
    std::list<struct queue_pairs> &thread_work = Singleton<struct LabStorContext>->per_thread_qps_[];
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("USAGE: ./server [config.yaml]");
        exit(1);
    }

    //Load a configuration file
    YAML::Node config = YAML::LoadFile(argv[1]);

    //Create general queue
    Singleton<struct LabStorContext>->admin_qp_->ceate_queue();

    //Load the polling function
    create_thread_pool(get_nprocs());
}