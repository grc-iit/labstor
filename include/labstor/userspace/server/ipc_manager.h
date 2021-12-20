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
#include <labstor/types/allocator/segment_allocator.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include "per_process_ipc.h"
#include <labstor/types/data_structures/unordered_map/shmem_unordered_map_int_PerProcessIPC.h>
#include <labstor/types/data_structures/unordered_map/shmem_unordered_map_labstor_qid_t_qp.h>
#include <labstor/types/thread_local.h>

#include "macros.h"
#include "server.h"

namespace labstor::Server {

struct MemoryConfig {
    uint32_t region_size;
    uint32_t request_unit;
    uint32_t min_request_region;
    uint32_t queue_depth;
    uint32_t num_queues;
    uint32_t queue_region_size;
    uint32_t request_region_size;
    uint32_t request_queue_size;
    uint32_t request_map_size;
};

class IPCManager {
private:
    int pid_;
    int server_fd_;
    void *private_mem_, *kern_base_region_;
    std::mutex lock_;
    std::vector<int> pids_;
    labstor::GenericAllocator *private_alloc_;
    labstor::ipc::mpmc::int_map_int_PerProcessIPC pid_to_ipc_;
    labstor::ipc::mpmc::int_map_labstor_qid_t_qp qps_by_id_;
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
public:
    IPCManager() {
        pid_ = getpid();
        labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
        uint32_t pid_to_ipc_size = labstor_config_->config_["ipc_manager"]["pid_to_ipc_size_mb"].as<uint32_t>()*SizeType::MB;
        uint32_t qps_by_id_size = labstor_config_->config_["ipc_manager"]["qps_by_id_size_mb"].as<uint32_t>()*SizeType::MB;
        uint32_t max_collisions = labstor_config_->config_["ipc_manager"]["max_collisions"].as<uint32_t>();

        void *base_region;
        pid_to_ipc_.Init(base_region = malloc(pid_to_ipc_size), base_region, pid_to_ipc_size, 16);
        qps_by_id_.Init(base_region = malloc(pid_to_ipc_size), base_region, qps_by_id_size, 16);
        pid_to_ipc_.Set(pid_, new PerProcessIPC());
    }
    ~IPCManager() {
        free(pid_to_ipc_.GetRegion());
        free(qps_by_id_.GetRegion());
    }

    inline int GetPid() {
        return pid_;
    }

    inline void SetServerFd(int fd) { server_fd_ = fd; }
    inline int GetServerFd() { return server_fd_; }

    void LoadMemoryConfig(std::string pid_type, MemoryConfig &config);
    void InitializeKernelIPCManager();
    void CreateKernelQueues();
    void CreatePrivateQueues();
    void RegisterClient(int client_fd, labstor::credentials &creds);
    void RegisterClientQP(PerProcessIPC *client_ipc, void *region);
    void PauseQueues();
    void WaitForPause();
    void ResumeQueues();

    inline bool RegisterQueuePair(labstor::ipc::queue_pair *qp) {
        //TODO: Thread safety? Not important now.
        ++pid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qp->GetQid())]->num_stream_qps_;
        return qps_by_id_.Set(qp->GetQid(), qp);
    }
    inline bool UnregisterQueuePair(labstor::ipc::queue_pair *qp) {
        return qps_by_id_.Remove(qp->GetQid());
    }
    inline void* GetRegion(int pid) {
        return pid_to_ipc_[pid]->GetRegion();
    }
    inline void* GetRegion(labstor::ipc::queue_pair *qp, labstor::credentials *&creds) {
        PerProcessIPC *ipc = pid_to_ipc_[qp->GetPID()];
        creds = &ipc->creds_;
        return ipc->GetRegion();
    }
    inline void* GetRegion(labstor::ipc::queue_pair *qp) {
        return pid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qp->GetQid())]->GetRegion();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_ipc_[pid_]->num_stream_qps_;
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, labstor::ThreadLocal::GetTid(), num_qps, pid_)];
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
        return GetQueuePairByPidHash(qp, flags, pid, labstor::ThreadLocal::GetTid());
    }
    inline void GetNextQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags) {
        return GetQueuePairByPidHash(qp, flags, pid_, labstor::ThreadLocal::GetTid() + 1);
    }
    inline void GetBatchQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qid_t flags, uint32_t depth) {
        if(LABSTOR_QP_IS_BATCH(flags)) {
            uint32_t sq_sz = labstor::ipc::request_queue::GetSize(depth);
            uint32_t cq_sz = labstor::ipc::request_map::GetSize(depth);
            qp->Init(flags,
                     private_alloc_->GetRegion(),
                     private_alloc_->Alloc(sq_sz), sq_sz,
                     private_alloc_->Alloc(cq_sz), cq_sz);
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, labstor::ipc::qtok_t &qtok) {
        qp = qps_by_id_[qtok.qid];
    }

    template<typename T>
    inline T* AllocRequest(labstor::ipc::qid_t qid, uint32_t size) {
        labstor::GenericAllocator* alloc = pid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qid)]->alloc_;
        return (T*)alloc->Alloc(size, labstor::ThreadLocal::GetTid());
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
        labstor::GenericAllocator* alloc = pid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qid)]->alloc_;
        alloc->Free((void*)rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qtok_t &qtok, T *rq) {
        return FreeRequest(qtok.qid, rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::queue_pair *qp, T *rq) {
        labstor::GenericAllocator* alloc = pid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qp->GetQid())]->alloc_;
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
