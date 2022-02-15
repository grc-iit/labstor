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
#include <labstor/types/thread_local.h>
#include <labstor/types/data_structures/mpmc/unordered_map/shmem_int_map.h>

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
    uint16_t lpid_;
    int server_fd_;
    void *private_mem_, *kern_base_region_;
    std::mutex lock_;
    std::vector<int> pids_;
    labstor::GenericAllocator *private_alloc_;
    labstor::ipc::mpmc::ring_buffer<uint16_t> lpids_;
    std::unordered_map<uint32_t,uint32_t> pid_to_lpid_;
    std::vector<PerProcessIPC*> lpid_to_ipc_;
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
public:
    IPCManager() {
        pid_ = getpid();
        labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
        uint32_t max_procs = labstor_config_->config_["ipc_manager"]["max_connected_procs"].as<uint32_t>();

        //Cache available LPIDs
        uint32_t region_size = labstor::ipc::mpmc::ring_buffer<uint16_t>::GetSize(max_procs);
        lpids_.Init(malloc(region_size), region_size);
        for(int i = 0; i < max_procs; ++i) {
            lpids_.Enqueue(i);
        }
        lpid_to_ipc_.resize(max_procs);

        //Initialize self lpid
        RegisterIPC(pid_, lpid_);
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

    PerProcessIPC* RegisterIPC(int pid, uint16_t &lpid) {
        PerProcessIPC *ipc = new PerProcessIPC();
        lpids_.Dequeue(lpid);
        pid_to_lpid_.emplace(pid, lpid);
        return ipc;
    }
    int GetLPID(int pid) {
        return pid_to_lpid_[pid];
    }
    inline bool RegisterQueuePair(labstor::ipc::queue_pair *qp) {
        //TODO: Thread safety? Not important now
        ++lpid_to_ipc_[qp->GetQid().lpid_]->num_stream_qps_;
        return true;
    }
    inline bool UnregisterQueuePair(labstor::ipc::queue_pair *qp) {
        return false;
    }
    /*inline void* GetRegion(int pid) {
        return pid_to_ipc_[pid]->GetRegion();
    }*/
    inline void* GetRegion(labstor::ipc::queue_pair *qp, labstor::credentials *&creds) {
        PerProcessIPC *ipc = pid_to_ipc_[qp->GetPID()];
        creds = &ipc->creds_;
        return ipc->GetRegion();
    }
    inline void* GetRegion(labstor::ipc::queue_pair *qp) {
        return lpid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qp->GetQid())]->GetRegion();
    }
    inline void GetQueuePair(labstor::ipc::queue_pair *&qp, uint16_t flags) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = lpid_to_ipc_[lpid_]->num_stream_qps_;
            labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(flags, labstor::ThreadLocal::GetTid(), num_qps, pid_);
            qp = lpid_to_ipc_[qid.lpid_]->GetQueuePair(qid);
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePairByName(labstor::ipc::queue_pair *&qp, uint16_t flags, const std::string &str, uint32_t ns_id) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = lpid_to_ipc_[lpid_]->num_stream_qps_;
            labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(flags, str, ns_id, num_qps, lpid_);
            qp = lpid_to_ipc_[qid.lpid_]->GetQueuePair(qid);
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePairByLpidHash(labstor::ipc::queue_pair *&qp, uint16_t flags, int lpid, int hash) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = lpid_to_ipc_[lpid]->num_stream_qps_;
            labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(flags, hash, num_qps, lpid);
            qp = lpid_to_ipc_[qid.lpid_]->GetQueuePair(qid);
            return;
        }
        throw INVALID_QP_QUERY.format();
    }
    inline void GetQueuePairByHash(labstor::ipc::queue_pair *&qp, uint16_t flags, int hash) {
        return GetQueuePairByLpidHash(qp, flags, lpid_, hash);
    }
    inline void GetQueuePairByPid(labstor::ipc::queue_pair *&qp, uint16_t flags, int lpid) {
        return GetQueuePairByLpidHash(qp, flags, lpid, labstor::ThreadLocal::GetTid());
    }
    inline void GetNextQueuePair(labstor::ipc::queue_pair *&qp, uint16_t flags) {
        return GetQueuePairByLpidHash(qp, flags, lpid_, labstor::ThreadLocal::GetTid() + 1);
    }
    inline void GetBatchQueuePair(labstor::ipc::queue_pair *&qp, uint16_t flags, uint32_t depth) {
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
        labstor::ipc::qid_t qid = qp->GetQid();
        qp = lpid_to_ipc_[qid.lpid_]->GetQueuePair(qid);
    }

    template<typename T>
    inline T* AllocRequest(labstor::ipc::qid_t qid, uint32_t size) {
        labstor::GenericAllocator* alloc = lpid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qid)]->alloc_;
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
        labstor::GenericAllocator* alloc = lpid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qid)]->alloc_;
        alloc->Free((void*)rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qtok_t &qtok, T *rq) {
        return FreeRequest(qtok.qid_, rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::queue_pair *qp, T *rq) {
        labstor::GenericAllocator* alloc = lpid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qp->GetQid())]->alloc_;
        alloc->Free((void*)rq);
    }

    template<typename T>
    inline T* Wait(labstor::ipc::qtok_t &qtok) {
        T *rq;
        labstor::ipc::queue_pair *qp;
        GetQueuePair(qp, qtok);
        rq = qp->Wait<T>(qtok.req_id_);
        return rq;
    }

    template<typename T>
    inline int Wait(labstor::ipc::qtok_set &qtoks) {
        AUTO_TRACE("")
        labstor::ipc::qtok_t qtok;
        while(qtoks.Dequeue(qtok)) {
            T *rq = Wait<T>(qtok);
            //TODO: Check if request successful
            FreeRequest(qtok, rq);
        }
        return LABSTOR_REQUEST_SUCCESS;
    }
    inline std::vector<int> &GetConnectedProcesses() {
        return pids_;
    }
    inline PerProcessIPC* GetIPC(int pid) {
        return lpid_to_ipc_[pid];
    }
};

}
#endif //LABSTOR_SERVER_IPCMANAGER_H
