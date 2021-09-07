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
#include <memory>

#include <labstor/util/singleton.h>
#include <ptrhead>
#include <sys/socket.h>

#include <unordered_map>
#include <list>

#define TRUSTED_SERVER_PATH "/tmp/server"

struct connection_data {
    int sockfd_;
    struct ucred ucred_;
    std::vector<labstor::ipc::queue_pair> qps_;
};

struct LabStorContext {
    labstor::ipc::unordered_map<std::string, struct pkg> namespace_;
    std::unordered_map<pid_t, connection_data> sockets_;
    std::unordered_map<labstor_id, labstor::pkg *
    std::vector<int> workers_;
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

void poll_request_queues(void *nothing) {
}

void allocate_queues(pid_t process) {
}

void server_loop(void *nothing) {
}

void accept_server_inits(void *nothing) {
    int ret;
    struct ucred ucred;
    int len;
    while(1) {
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }
        len = sizeof(struct ucred);
        ret = getsockopt(sock, SOL_SOCKET, SO_PEERCRED, &ucred, &len);
        if (ret < 0) {
            perror("ERROR on acquiring client's credentials");
            exit(1);
        }
        std::unordered_map<pid_t pid, std::shared_ptr<connection_data>> &sockets = Singleton<LabStorContext>->sockets_;
        if(sockets.find(id) != sockets.end()) {
            sockets[pid] = std::shared_ptr<connection_data>(new connection_data(sockfd, ucred))>;
        }
    }
}

void server_init(void) {
    int id_;
    int sockfd_;
    pthread_t accept_thread_;
    pthread_t loop_thread_;
    pthread_attr_t attr_ = 1;

    int ret;
    struct sockaddr_un serveraddr;

    id_ = id;
    sockfd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sockfd_ < 0) {
        perror("socket() failed");
        return;
    }

    ret = setsockopt(sockfd_, SOL_SOCKET, SO_PASSCRED, (void*)&attr_, sizeof(attr_));
    if(ret < 0) {
        perror("setsockopt() failed");
        return;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, TRUSTED_SERVER_PATH);
    ret = bind(sockfd_, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
    if(ret < 0) {
        perror("bind() failed");
        return;
    }

    ret = listen(sockfd_, 1024);
    if(ret < 0) {
        perror("listen() failed");
        return;
    }

    pthread_create(&accept_thread_, );
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("USAGE: ./server [config.yaml]");
        exit(1);
    }

    //Load a configuration file
    YAML::Node config = YAML::LoadFile(argv[1]);

    //Create initialization server
    server_init();

    //Load the polling function
    create_thread_pool(get_nprocs());
}

//https://davejingtian.org/2015/02/17/retrieve-pid-from-the-packet-in-unix-domain-socket-a-complete-use-case-for-recvmsgsendmsg/
//https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/storage_administration_guide/configuring-persistent-memory-for-use-in-device-dax-mode