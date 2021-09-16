//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <pthread.h>
#include <memory>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sysinfo.h>
#include <sched.h>

#include "server.h"
#include <labstor/types/basics.h>
#include <labstor/util/singleton.h>
#include <labstor/userspace_server/worker.h>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

void create_thread_pool(int n_cpu, int threads_per_cpu) {
    auto labstor_context = scs::Singleton<labstor::LabStorServerContext>::GetInstance();
    int nthreads = n_cpu * threads_per_cpu;
    cpu_set_t cpus[n_cpu];
    CPU_ZERO(cpus);

    //Work orchestrator thread (none)
    //pthread_create(&scs::Singleton<labstor::LabStorServerContext>::GetInstance()->work_orchestrator_, );

    //Worker threads
    labstor_context->worker_pool_.reserve(nthreads);
    for(int i = 0; i < nthreads; ++i) {
        CPU_SET((i%n_cpu), cpus);
        pthread_create(&scs::Singleton<labstor::LabStorServerContext>::GetInstance()->worker_pool_[i], NULL, worker, NULL);
        pthread_setaffinity_np(scs::Singleton<labstor::LabStorServerContext>::GetInstance()->worker_pool_[i], n_cpu, cpus);
        CPU_CLR((i%n_cpu), cpus);
    }
}

void* accept_initial_connections(void *nothing) {
    auto labstor_context = scs::Singleton<labstor::LabStorServerContext>::GetInstance();
    int ret;
    struct ucred ucred;
    struct labstor::credentials creds;
    int server_fd_ = labstor_context->accept_fd_;
    int client_fd_;
    struct sockaddr_un client_addr_;
    struct labstor::setup_request client_hints_;
    socklen_t clilen, len;

    while(1) {
        //Accept client connection
        client_fd_ = accept(server_fd_, (struct sockaddr *) &client_addr_, &clilen);
        if (client_fd_ < 0) {
            perror("ERROR on accept");
            exit(1);
        }
        printf("New client was accepted!\n");
        //Get the client's credentials
        len = sizeof(struct ucred);
        ret = getsockopt(client_fd_, SOL_SOCKET, SO_PEERCRED, &ucred, &len);
        if (ret < 0) {
            perror("ERROR on acquiring client's credentials");
            exit(1);
        }
        memcpy(&creds, &ucred, sizeof(ucred));
        printf("New client (pid=%d uid=%d gid=%d) was accepted!\n", creds.pid, creds.uid, creds.gid);
        //Receive message containing thread and queue size hints
        recv(client_fd_, (void*)&client_hints_, sizeof(client_hints_), 0);
        //Create new SHMEM queues
        labstor_context->ipc_manager_.CreateIPC(client_fd_, &creds, client_hints_.num_queues, client_hints_.queue_size);
        //Resend the same message to the server
        send(client_fd_, (void*)&client_hints_, sizeof(client_hints_), 0);
    }
}

void server_init(void) {
    int server_fd_;
    int optval = 1;
    int ret;
    struct sockaddr_un server_addr_;
    auto labstor_context = scs::Singleton<labstor::LabStorServerContext>::GetInstance();

    remove(TRUSTED_SERVER_PATH);

    server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    labstor_context->accept_fd_ = server_fd_;
    if(server_fd_ < 0) {
        perror("socket() failed");
        return;
    }

    ret = setsockopt(server_fd_, SOL_SOCKET, SO_PASSCRED, (void*)&optval, sizeof(optval));
    if(ret < 0) {
        perror("setsockopt() failed");
        return;
    }

    memset(&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sun_family = AF_UNIX;
    strcpy(server_addr_.sun_path, TRUSTED_SERVER_PATH);
    ret = bind(server_fd_, (struct sockaddr *)&server_addr_, SUN_LEN(&server_addr_));
    if(ret < 0) {
        perror("bind() failed");
        return;
    }

    ret = listen(server_fd_, 1024);
    if(ret < 0) {
        perror("listen() failed");
        return;
    }

    //pthread_create(&labstor_context->accept_thread_, NULL, accept_initial_connections, NULL);
    accept_initial_connections(nullptr);
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("USAGE: ./server [config.yaml]\n");
        exit(1);
    }
    auto labstor_context = scs::Singleton<labstor::LabStorServerContext>::GetInstance();

    //Load a configuration file
    //YAML::Node config = YAML::LoadFile(argv[1]);

    //Get this process' info
    labstor_context->pid_ = getpid();

    //Connect to the kernel server and establish IPCs

    //Load all modules

    //Load the workers
    create_thread_pool(get_nprocs_conf(), 1);

    //Create initialization server
    printf("LabStor Trusted Server running!\n");
    server_init();
}

//https://davejingtian.org/2015/02/17/retrieve-pid-from-the-packet-in-unix-domain-socket-a-complete-use-case-for-recvmsgsendmsg/
//https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/storage_administration_guide/configuring-persistent-memory-for-use-in-device-dax-mode
