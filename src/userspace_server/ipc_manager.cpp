//
// Created by lukemartinlogan on 9/7/21.
//

#include <memory>

#include <labstor/util/errors.h>
#include <labstor/util/singleton.h>
#include <labstor/types/basics.h>
#include <labstor/types/socket.h>
#include <labstor/userspace_server/server.h>
#include <labstor/userspace_server/ipc_manager.h>
#include <labstor/kernel_client/kernel_client.h>
#include <labstor/types/messages.h>

void labstor::Server::IPCManager::RegisterClient(int client_fd, labstor::credentials &creds) {
    lock_.lock();
    //Create new IPC
    pid_to_ipc_.emplace(creds.pid, PerProcessIPC(client_fd, creds));
    PerProcessIPC &ipc = pid_to_ipc_[creds.pid];
    //Create shared memory
    lock_.unlock();

    //Send shared memory to client
    labstor::ipc::setup_reply reply;
    reply.region_id = 0;
    reply.region_size = labstor_config_->config_["ipc_manager"]["process_shmem_kb"].as<int>();
    ipc.GetSocket().SendMSG(&reply, sizeof(reply));
}

void labstor::Server::IPCManager::RegisterQP(PerProcessIPC client_ipc, labstor::ipc::admin_request header) {
    //Receive SHMEM queue offsets
    labstor::ipc::register_qp_request request;
    client_ipc.GetSocket().RecvMSG((void*)&request, sizeof(labstor::ipc::register_qp_request));

    //Receive the SHMEM queue pointers
    uint32_t size = request.count_*sizeof(labstor::ipc::queue_pair_ptr);
    labstor::ipc::queue_pair_ptr *ptrs = (labstor::ipc::queue_pair_ptr*)malloc(size);
    client_ipc.GetSocket().RecvMSG((void*)ptrs, size);
    for(int i = 0; i < request.count_; ++i) {
    }
    free(ptrs);

    //Reply success
    labstor::ipc::register_qp_reply reply(0);
    client_ipc.GetSocket().SendMSG((void*)&reply, sizeof(labstor::ipc::register_qp_reply));
}

void labstor::Server::IPCManager::PauseQueues() {
    for(auto &pid_ipc : pid_to_ipc_) {
        PerProcessIPC &ipc = pid_ipc.second;
        for(labstor::ipc::queue_pair &qp : ipc.qps_) {
            if(LABSTOR_QP_IS_PRIMARY(qp.sq.GetFlags())) {
                qp.sq.MarkPaused();
            }
        }
    }
}

void labstor::Server::IPCManager::WaitForPause() {
    int num_unpaused;
    do {
        num_unpaused = 0;
        for (auto &pid_ipc : pid_to_ipc_) {
            PerProcessIPC &ipc = pid_ipc.second;
            for (labstor::ipc::queue_pair &qp : ipc.qps_) {
                if(LABSTOR_QP_IS_PRIMARY(qp.sq.GetFlags())) {
                    num_unpaused += qp.sq.IsPaused();
                } else {
                    num_unpaused += qp.sq.GetDepth();
                }
            }
        }
    } while(num_unpaused);
}

void labstor::Server::IPCManager::ResumeQueues() {
    for(auto &pid_ipc : pid_to_ipc_) {
        PerProcessIPC &ipc = pid_ipc.second;
        for(labstor::ipc::queue_pair &qp : ipc.qps_) {
            if(LABSTOR_QP_IS_PRIMARY(qp.sq.GetFlags())) {
                qp.sq.UnPause();
            }
        }
    }
}