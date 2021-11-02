//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <memory>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sysinfo.h>
#include <sched.h>

#include <labstor/userspace_server/server.h>
#include <labstor/types/basics.h>
#include <labstor/util/singleton.h>
#include <labstor/userspace_server/worker.h>
#include <labstor/kernel_client/kernel_client.h>
#include <yaml-cpp/yaml.h>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

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
    auto labstor_kernel_context = scs::Singleton<labstor::LabStorKernelClientContext>::GetInstance();

    //Load a configuration file
    labstor_context->LoadConfig(argv[1]);

    //Get this process' info
    labstor_context->pid_ = getpid();

    //Connect to the kernel server and establish server-kernel SHMEM queues
    //labstor_kernel_context->Connect();

    //Load all modules
    for (const auto &module : labstor_context->config_["modules"]) {
        labstor::id module_id(module.first.as<std::string>());
        labstor::ModulePath paths;
        if(module.second["client"]) {
            paths.client = module.second["client"].as<std::string>();
        }
        if(module.second["trusted_client"]) {
            paths.trusted_client = module.second["trusted_client"].as<std::string>();
        }
        if(module.second["server"]) {
            paths.server = module.second["server"].as<std::string>();
        }
        labstor_context->module_manager_.AddModulePaths(module_id, paths);
        labstor_context->module_manager_.UpdateModule(paths.server);
    }

    //Load the workers
    labstor_context->work_orchestrator_.CreateWorkers();

    //Create initialization server
    printf("LabStor Trusted Server running!\n");
    server_init();
}
