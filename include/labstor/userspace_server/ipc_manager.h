//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SERVER_IPCMANAGER_H
#define LABSTOR_SERVER_IPCMANAGER_H

#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <mutex>

#include <labstor/constants/constants.h>
#include <labstor/types/messages.h>
#include <labstor/types/socket.h>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/allocator.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_int_map.h>

#include "macros.h"
#include "server.h"
#include "work_orchestrator.h"

namespace labstor::Server {

struct PerProcessIPC {
    UnixSocket clisock_;
    labstor::credentials creds_;
    int region_id_;
    void *shmem_region_;
    labstor::GenericAllocator *alloc_;
    int num_stream_qps_;

    PerProcessIPC() {}
    PerProcessIPC(int fd, labstor::credentials creds) : clisock_(fd), creds_(creds) {}
    PerProcessIPC(const PerProcessIPC &old_ipc) {
        clisock_ = old_ipc.clisock_;
        creds_ = old_ipc.creds_;
        shmem_region_ = old_ipc.shmem_region_;
        alloc_ = old_ipc.alloc_;
    }
    UnixSocket& GetSocket() { return clisock_; };
};

class IPCManager {
private:
    int pid_;
    int server_fd_;
    void *internal_mem_;
    std::mutex lock_;
    labstor::GenericAllocator *private_alloc_;
    std::vector<int> pids_;
    labstor::ipc::int_map<int, PerProcessIPC> pid_to_ipc_;
    labstor::ipc::int_map<uint32_t, labstor::ipc::queue_pair> qps_by_id_;
    ShmemNetlinkClient shmem_client_;
    uint32_t per_process_shmem_, allocator_unit_;
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
    LABSTOR_WORK_ORCHESTRATOR_T work_orchestrator_;
public:
    IPCManager() {
        pid_ = getpid();
        labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
        work_orchestrator_ = LABSTOR_WORK_ORCHESTRATOR;

        uint32_t pid_to_ipc_size = labstor_config_->config_["ipc_manager"]["pid_to_ipc_size_mb"].as<uint32_t>()*SizeType::MB;
        uint32_t qps_by_id_size = labstor_config_->config_["ipc_manager"]["qps_by_id_size_mb"].as<uint32_t>()*SizeType::MB;
        uint32_t max_collisions = labstor_config_->config_["ipc_manager"]["max_collisions"].as<uint32_t>();
        per_process_shmem_ = labstor_config_->config_["ipc_manager"]["process_shmem_kb"].as<uint32_t>()*SizeType::KB;
        allocator_unit_ = labstor_config_->config_["ipc_manager"]["allocator_unit_bytes"].as<uint32_t>()*SizeType::BYTES;

        pid_to_ipc_.Init(malloc(pid_to_ipc_size), pid_to_ipc_size, max_collisions);
        qps_by_id_.Init(malloc(qps_by_id_size), qps_by_id_size, max_collisions);
        pid_to_ipc_.Set(pid_, PerProcessIPC());
    }
    ~IPCManager() {
        free(pid_to_ipc_.GetRegion());
        free(qps_by_id_.GetRegion());
    }

    inline void SetServerFd(int fd) { server_fd_ = fd; }
    inline int GetServerFd() { return server_fd_; }

    void RegisterClient(int client_fd, labstor::credentials &creds);
    void RegisterQP(PerProcessIPC &client_ipc);
    void PauseQueues();
    void WaitForPause();
    void ResumeQueues();

    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_ipc_[pid_].num_stream_qps_;
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, sched_getcpu(), num_qps, pid_)];
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, int hash) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_ipc_[pid_].num_stream_qps_;
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, hash, num_qps, pid_)];
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, const std::string &str, uint32_t ns_id) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_ipc_[pid_].num_stream_qps_;
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, str, ns_id, num_qps, pid_)];
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, uint32_t depth=0, int pid=-1) {
        if(pid >= 0) {
            uint32_t num_qps = pid_to_ipc_[pid_].num_stream_qps_;
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, sched_getcpu(), num_qps, pid_)];
            return;
        }
        if(LABSTOR_QP_IS_BATCH(flags)) {
            uint32_t sq_sz = labstor::ipc::request_queue::GetSize(depth);
            uint32_t cq_sz = labstor::ipc::request_map::GetSize(depth, 4);
            qp.sq.Init(private_alloc_->Alloc(sq_sz), sq_sz, flags);
            qp.cq.Init(private_alloc_->Alloc(cq_sz), cq_sz, 4);
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, labstor::ipc::qtok_t &qtok) {
        qp = qps_by_id_[qtok.qid];
    }
    inline labstor::ipc::request* AllocRequest(uint32_t qid, uint32_t size) {
        labstor::GenericAllocator* alloc = pid_to_ipc_[LABSTOR_GET_QP_PID(qid)].alloc_;
        return (labstor::ipc::request*)alloc->Alloc(size);
    }
    inline labstor::ipc::request* AllocRequest(labstor::ipc::queue_pair &qp, uint32_t size) {
        return AllocRequest(qp.GetQid(), size);
    }
    inline void FreeRequest(uint32_t qid, labstor::ipc::request *rq) {
        labstor::GenericAllocator* alloc = pid_to_ipc_[LABSTOR_GET_QP_PID(qid)].alloc_;
        alloc->Free((void*)rq);
    }
    inline void FreeRequest(labstor::ipc::qtok_t &qtok, labstor::ipc::request *rq) {
        return FreeRequest(qtok.qid, rq);
    }
    inline labstor::ipc::request* Wait(labstor::ipc::qtok_t &qtok) {
        labstor::ipc::request *rq;
        labstor::ipc::queue_pair qp;
        GetQueuePair(qp, qtok);
        rq = qp.Wait(qtok.req_id);
        return rq;
    }
    inline void Wait(labstor::ipc::qtok_set &qtoks) {
        for(int i = 0; i < qtoks.GetLength(); ++i) {
            FreeRequest(qtoks[i], Wait(qtoks[i]));
        }
    }

    inline std::vector<int> &GetConnectedProcesses() {
        return pids_;
    }

    inline PerProcessIPC GetIPC(int pid) {
        return pid_to_ipc_[pid];
    }
};

}
#endif //LABSTOR_SERVER_IPCMANAGER_H
