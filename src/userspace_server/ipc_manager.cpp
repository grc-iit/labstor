//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/util/singleton.h>
#include <labstor/types/basics.h>
#include <labstor/userspace_server/ipc_manager.h>
#include "server.h"

void* labstor::IPCManager::WreapProcesses(void *nothing) {
    auto labstor_context = scs::Singleton<labstor::LabStorServerContext>::GetInstance();
    while(1) {
        //Iterate over all IPCs and then check if process is still alive
        //If dead, release shared memory
    }
}

void labstor::IPCManager::CreateIPC(int client_fd, labstor::credentials *creds, int count, size_t size) {
    auto labstor_context = scs::Singleton<labstor::LabStorServerContext>::GetInstance();
    auto &ipc_pool = labstor_context->ipc_pool_;
    if(ipc_pool.find(creds->pid) != ipc_pool.end()) { return; }
    auto self_ipc_ = ipc_pool[0];

    //Allocate SHMEM queues
    /*for(int i = 0; i < count; ++i) {
        struct kernel_request *rq = (struct kernel_request *)self_ipc_->GetRequest();
        rq->open.path = pathname;
        rq->open.flags = flags;
        rq->open.mode = mode;
        qp->StartRequest();
    }
    qp->Wait();*/

    //Register the IPC in the table
    auto proc_ipc_ = std::shared_ptr<labstor::PerProcessIPC>(new labstor::PerProcessIPC(client_fd, creds));
    //Add queues to table
    ipc_pool[creds->pid] = proc_ipc_;
}