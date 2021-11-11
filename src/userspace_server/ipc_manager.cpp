//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/util/singleton.h>
#include <labstor/types/basics.h>
#include <labstor/userspace_server/server.h>
#include <labstor/userspace_server/ipc_manager.h>
#include <labstor/kernel_client/kernel_client.h>

void* labstor::Server::IPCManager::WreapProcesses(void *nothing) {
    auto labstor_config_ = scs::Singleton<labstor::Server::ConfigurationManager>::GetInstance();
    while(1) {
        //Iterate over all IPCs and then check if process is still alive
        //If dead, release shared memory
    }
}

void labstor::Server::IPCManager::RegisterClient(int client_fd, labstor::credentials *creds, int num_queues) {
    register_lock_.lock();
    register_lock_.unlock();
}

labstor::ipc::queue_pair& GetQP(int flags) {
}

labstor::ipc::queue_pair& GetQP(int nreqs, int req_size, int flags) {
}