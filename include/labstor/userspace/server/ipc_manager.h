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
#include <labstor/userspace/types/messages.h>
#include <labstor/userspace/types/socket.h>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/allocator.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include "per_process_ipc.h"
#include <labstor/types/data_structures/shmem_unordered_map_int_PerProcessIPC.h>
#include <labstor/types/data_structures/shmem_unordered_map_labstor_qid_t_qp.h>

#include "macros.h"
#include "server.h"

namespace labstor::Server {

class IPCManager {
private:
    int pid_;
    int server_fd_;
    void *private_mem_;
    std::mutex lock_;
    labstor::GenericAllocator *private_alloc_;
    std::vector<int> pids_;
    labstor::ipc::int_map_int_PerProcessIPC pid_to_ipc_;
    labstor::ipc::int_map_labstor_qid_t_qp qps_by_id_;
    uint32_t per_process_shmem_, allocator_unit_;
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
public:
    IPCManager() {
        pid_ = getpid();
        labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
        uint32_t pid_to_ipc_size = labstor_config_->config_["ipc_manager"]["pid_to_ipc_size_mb"].as<uint32_t>()*SizeType::MB;
        uint32_t qps_by_id_size = labstor_config_->config_["ipc_manager"]["qps_by_id_size_mb"].as<uint32_t>()*SizeType::MB;
        uint32_t max_collisions = labstor_config_->config_["ipc_manager"]["max_collisions"].as<uint32_t>();
        per_process_shmem_ = labstor_config_->config_["ipc_manager"]["process_shmem_kb"].as<uint32_t>()*SizeType::KB;
        allocator_unit_ = labstor_config_->config_["ipc_manager"]["allocator_unit_bytes"].as<uint32_t>()*SizeType::BYTES;

        void *base_region;
        pid_to_ipc_.Init(base_region = malloc(pid_to_ipc_size), base_region, pid_to_ipc_size, max_collisions);
        qps_by_id_.Init(base_region = malloc(pid_to_ipc_size), base_region, qps_by_id_size, max_collisions);
        pid_to_ipc_.Set(pid_, new PerProcessIPC());
    }
    ~IPCManager() {
        free(pid_to_ipc_.GetRegion());
        free(qps_by_id_.GetRegion());
    }

    inline void SetServerFd(int fd) { server_fd_ = fd; }
    inline int GetServerFd() { return server_fd_; }

    void InitializeKernelIPCManager();
    void CreateKernelQueues();
    void CreatePrivateQueues();
    void RegisterClient(int client_fd, labstor::credentials &creds);
    void RegisterClientQP(PerProcessIPC *client_ipc);
    void PauseQueues();
    void WaitForPause();
    void ResumeQueues();

    inline bool RegisterQueuePair(labstor::ipc::queue_pair *qp) {
        //TODO: Thread safety? Not important now.
        ++pid_to_ipc_[LABSTOR_GET_QP_PID(qp->GetQid())]->num_stream_qps_;
        return qps_by_id_.Set(qp->GetQid(), qp);
    }
    inline bool UnregisterQueuePair(labstor::ipc::queue_pair *qp) {
        return qps_by_id_.Remove(qp->GetQid());
    }
    inline void* GetRegion(int pid) {
        return pid_to_ipc_[pid]->GetRegion();
    }
    inline void* GetRegion(labstor::ipc::queue_pair_ptr &qp_ptr, labstor::credentials *&creds) {
        PerProcessIPC *ipc = pid_to_ipc_[qp_ptr.GetPID()];
        creds = &ipc->creds_;
        return ipc->GetRegion();
    }
    inline void* GetRegion(labstor::ipc::queue_pair *qp) {
        return pid_to_ipc_[LABSTOR_GET_QP_PID(qp->GetQid())]->GetRegion();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_ipc_[pid_]->num_stream_qps_;
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, sched_getcpu(), num_qps, pid_)];
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePairByName(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags, const std::string &str, uint32_t ns_id) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_ipc_[pid_]->num_stream_qps_;
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, str, ns_id, num_qps, pid_)];
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePairByPidHash(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags, int pid, int hash) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_ipc_[pid]->num_stream_qps_;
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, hash, num_qps, pid)];
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePairByHash(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags, int hash) {
        return GetQueuePairByPidHash(qp, flags, pid_, hash);
    }
    inline void GetQueuePairByPid(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags, int pid) {
        return GetQueuePairByPidHash(qp, flags, pid, sched_getcpu());
    }
    inline void GetNextQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags) {
        return GetQueuePairByPidHash(qp, flags, pid_, sched_getcpu() + 1);
    }
    inline void GetBatchQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags, uint32_t depth) {
        if(LABSTOR_QP_IS_BATCH(flags)) {
            uint32_t sq_sz = labstor::ipc::request_queue::GetSize(depth);
            uint32_t cq_sz = labstor::ipc::request_map::GetSize(depth, 4);
            qp->Init(flags, private_alloc_->GetRegion(), private_alloc_->Alloc(sq_sz), sq_sz, private_alloc_->Alloc(cq_sz), cq_sz);
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qtok_t &qtok) {
        qp = qps_by_id_[qtok.qid];
    }

    template<typename T>
    inline T* AllocRequest(labstor::ipc::qid_t qid, uint32_t size) {
        labstor::GenericAllocator* alloc = pid_to_ipc_[LABSTOR_GET_QP_PID(qid)]->alloc_;
        return (T*)alloc->Alloc(size);
    }
    template<typename T>
    inline T* AllocRequest(labstor::ipc::queue_pair *qp, uint32_t size) {
        return AllocRequest<T>(qp->GetQid(), size);
    }
    template<typename T>
    inline T* AllocRequest(labstor::ipc::queue_pair *qp) {
        return AllocRequest<T>(qp->GetQid(), sizeof(T));
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qid_t qid, T *rq) {
        labstor::GenericAllocator* alloc = pid_to_ipc_[LABSTOR_GET_QP_PID(qid)]->alloc_;
        alloc->Free((void*)rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qtok_t &qtok, T *rq) {
        return FreeRequest(qtok.qid, rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::queue_pair *qp, T *rq) {
        labstor::GenericAllocator* alloc = pid_to_ipc_[LABSTOR_GET_QP_PID(qp->GetQid())]->alloc_;
        alloc->Free((void*)rq);
    }

    template<typename T>
    inline T* Wait(labstor::ipc::qtok_t &qtok) {
        T *rq;
        labstor::ipc::queue_pair *qp;
        GetQueuePair(qp, qtok);
        rq = qp->Wait<T>(qtok.req_id);
        return rq;
    }

    template<typename T>
    inline void Wait(labstor::ipc::qtok_set &qtoks) {
        for(int i = 0; i < qtoks.GetLength(); ++i) {
            FreeRequest(qtoks[i], Wait<T>(qtoks[i]));
        }
    }
    inline std::vector<int> &GetConnectedProcesses() {
        return pids_;
    }
    inline PerProcessIPC* GetIPC(int pid) {
        return pid_to_ipc_[pid];
    }
};

}
#endif //LABSTOR_SERVER_IPCMANAGER_H
