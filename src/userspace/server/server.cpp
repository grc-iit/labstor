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
#include <yaml-cpp/yaml.h>

#include <labstor/types/basics.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/util/debug.h>

#include <labstor/userspace/types/userspace_daemon.h>
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/admin_thread.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/work_orchestrator.h>
#include <labstor/kernel/client/kernel_client.h>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

class AcceptWorker : public labstor::DaemonWorker {
private:
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    int server_fd_;
public:
    AcceptWorker() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        server_fd_ = ipc_manager_->GetServerFd();
    }

    void DoWork() {
        AUTO_TRACE("labstor::Server::AcceptWorker")
        int ret;
        struct ucred ucred;
        labstor::credentials creds;
        struct sockaddr_un client_addr_;
        socklen_t clilen, len;
        int client_fd_;

        LABSTOR_ERROR_HANDLE_START()

        //Accept client connection
        client_fd_ = accept(server_fd_, (struct sockaddr *) &client_addr_, &clilen);
        if (client_fd_ < 0) {
            throw labstor::UNIX_ACCEPT_FAILED.format(strerror(errno));
        }

        printf("New client was accepted!\n");
        //Get the client's credentials
        len = sizeof(struct ucred);
        ret = getsockopt(client_fd_, SOL_SOCKET, SO_PEERCRED, &ucred, &len);
        if (ret < 0) {
            throw labstor::UNIX_GETSOCKOPT_FAILED.format(strerror(errno));
        }
        memcpy(&creds, &ucred, sizeof(ucred));
        printf("New client (pid=%d uid=%d gid=%d) was accepted!\n", creds.pid, creds.uid, creds.gid);
        ipc_manager_->RegisterClient(client_fd_, creds);

        LABSTOR_ERROR_HANDLE_END()
    }
};

void server_init(void) {
    AUTO_TRACE("labstor::Server::server_init")
    int server_fd;
    int optval = 1;
    int ret;
    struct sockaddr_un server_addr_;
    auto labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    auto ipc_manager_ = LABSTOR_IPC_MANAGER;

    remove(TRUSTED_SERVER_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    ipc_manager_->SetServerFd(server_fd);
    if(server_fd < 0) {
        throw labstor::UNIX_SOCKET_FAILED.format(strerror(errno));
    }

    ret = setsockopt(server_fd, SOL_SOCKET, SO_PASSCRED, (void*)&optval, sizeof(optval));
    if(ret < 0) {
        throw labstor::UNIX_SETSOCKOPT_FAILED.format(strerror(errno));
    }

    memset(&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sun_family = AF_UNIX;
    strcpy(server_addr_.sun_path, TRUSTED_SERVER_PATH);
    ret = bind(server_fd, (struct sockaddr *)&server_addr_, SUN_LEN(&server_addr_));
    if(ret < 0) {
        throw labstor::UNIX_BIND_FAILED.format(strerror(errno));
    }

    ret = listen(server_fd, 1024);
    if(ret < 0) {
        throw labstor::UNIX_LISTEN_FAILED.format(strerror(errno));
    }
}

int main(int argc, char **argv) {
    AUTO_TRACE("labstor::Server")
    if(argc != 2) {
        printf("USAGE: ./server [config.yaml]\n");
        exit(1);
    }

    LABSTOR_ERROR_HANDLE_START()

    //Initialize labstor configuration
    auto labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    labstor_config_->LoadConfig(argv[1]);

    //Connect to kernel server
    auto netlink_client_ = LABSTOR_KERNEL_CLIENT;
    netlink_client_->Connect();

    //Load modules
    auto module_manager_ = LABSTOR_MODULE_MANAGER;
    module_manager_->LoadDefaultModules();

    //Initialize namespace
    auto namespace_ = LABSTOR_NAMESPACE;
    namespace_->Init();

    //Initialize workers
    auto work_orchestrator_ = LABSTOR_WORK_ORCHESTRATOR;
    work_orchestrator_->CreateWorkers();

    //Initialize server
    server_init();

    //Create the thread for accepting connections
    std::shared_ptr<labstor::UserspaceDaemon> accept_daemon = std::shared_ptr<labstor::UserspaceDaemon>(new labstor::UserspaceDaemon());
    std::shared_ptr<AcceptWorker> accept_worker = std::shared_ptr<AcceptWorker>(new AcceptWorker());
    accept_daemon->SetWorker(accept_worker);
    accept_daemon->Start();
    accept_daemon->SetAffinity(labstor_config_->config_["admin_thread"].as<int>());

    //Create the thread for processing administrative requests
    std::shared_ptr<labstor::UserspaceDaemon> admin_daemon = std::shared_ptr<labstor::UserspaceDaemon>(new labstor::UserspaceDaemon());
    std::shared_ptr<labstor::Server::AdminWorker> admin_worker = std::shared_ptr<labstor::Server::AdminWorker>(new labstor::Server::AdminWorker());
    admin_daemon->SetWorker(admin_worker);
    admin_daemon->Start();
    admin_daemon->SetAffinity(labstor_config_->config_["admin_thread"].as<int>());

    //Wait for the daemons to die
    printf("LabStor server has started\n");
    accept_daemon->Wait();
    admin_daemon->Wait();

    LABSTOR_ERROR_HANDLE_END()
}
