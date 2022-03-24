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
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include "per_process_ipc.h"
#include <labstor/types/thread_local.h>
#include <labstor/types/data_structures/unordered_map/shmem_int_map.h>

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
    labstor::GenericAllocator *private_alloc_;
    std::unordered_map<uint32_t,PerProcessIPC*> pid_to_ipc_;
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
public:
    IPCManager() {
        pid_ = getpid();
        labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
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

    int GetPID() {
        return pid_;
    }
    PerProcessIPC* RegisterIPC(int pid) {
        PerProcessIPC *ipc = new PerProcessIPC(pid);
        pid_to_ipc_.emplace(pid, ipc);
        return ipc;
    }
    PerProcessIPC* RegisterIPC(int fd, labstor::credentials &creds) {
        PerProcessIPC *ipc = new PerProcessIPC(fd, creds);
        pid_to_ipc_.emplace(creds.pid_, ipc);
        return ipc;
    }
    inline bool RegisterQueuePair(labstor::queue_pair *qp) {
        //TODO: Thread safety? Not important now
        TRACEPOINT("pid", qp->GetQID().pid_, "pid", qp->GetQID().type_, "flags", qp->GetQID().flags_, "cnt", qp->GetQID().cnt_)
        pid_to_ipc_[qp->GetQID().pid_]->RegisterQueuePair(qp);
        return true;
    }
    inline bool UnregisterQueuePair(labstor::queue_pair *qp) {
        //Todo: Dynamic queue management
        return false;
    }
    inline void* GetRegion(labstor::queue_pair *qp, labstor::credentials *&creds) {
        PerProcessIPC *ipc = pid_to_ipc_[qp->GetPID()];
        creds = &ipc->creds_;
        return ipc->GetRegion();
    }
    inline void* GetRegion(labstor::queue_pair *qp) {
        return pid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qp->GetQID())]->GetRegion();
    }

    inline void GetQueuePair(labstor::queue_pair *&qp, labstor_qid_type_t type, labstor_qid_flags_t flags, int cnt) {
        auto &ipc = pid_to_ipc_[pid_];
        labstor::ipc::qid_t qid = labstor::queue_pair::GetQID(type, flags, labstor::ThreadLocal::GetTid(), ipc->GetNumQueuePairsFast(0, flags), pid_);
        pid_to_ipc_[qid.pid_]->GetQueuePair(qp, qid);
    }
    inline void GetQueuePair(labstor::queue_pair *&qp, labstor_qid_flags_t flags) {
        auto &ipc = pid_to_ipc_[pid_];
        labstor::ipc::qid_t qid = labstor::queue_pair::GetQID(0, flags, labstor::ThreadLocal::GetTid(), ipc->GetNumQueuePairsFast(0, flags), pid_);
        pid_to_ipc_[qid.pid_]->GetQueuePair(qp, qid);
    }
    inline void GetQueuePairByName(labstor::queue_pair *&qp, labstor_qid_flags_t flags, const std::string &str, uint32_t ns_id) {
        auto &ipc = pid_to_ipc_[pid_];
        labstor::ipc::qid_t qid = labstor::queue_pair::GetQID(0, flags, str, ns_id, ipc->GetNumQueuePairsFast(0, flags), pid_);
        ipc->GetQueuePair(qp, qid);
    }
    inline void GetQueuePairByPidHash(labstor::queue_pair *&qp, labstor_qid_flags_t flags, int pid, int hash) {
        auto &ipc = pid_to_ipc_[pid];
        labstor::ipc::qid_t qid = labstor::queue_pair::GetQID(0, flags, hash, ipc->GetNumQueuePairsFast(0, flags), pid);
        ipc->GetQueuePair(qp, qid);
    }
    inline void GetQueuePairByHash(labstor::queue_pair *&qp, labstor_qid_flags_t flags, int hash) {
        GetQueuePairByPidHash(qp, flags, pid_, hash);
    }
    inline void GetQueuePairByPid(labstor::queue_pair *&qp, labstor_qid_flags_t flags, int pid) {
        GetQueuePairByPidHash(qp, flags, pid, labstor::ThreadLocal::GetTid());
    }
    inline void GetNextQueuePair(labstor::queue_pair *&qp, labstor_qid_flags_t flags) {
        GetQueuePairByPidHash(qp, flags, pid_, labstor::ThreadLocal::GetTid() + 1);
    }
    inline void GetQueuePair(labstor::queue_pair *&qp, labstor::ipc::qid_t &qid) {
        pid_to_ipc_[qid.pid_]->GetQueuePair(qp, qid);
    }
    inline void GetQueuePair(labstor::queue_pair *&qp, labstor::ipc::qtok_t &qtok) {
        GetQueuePair(qp, qtok.qid_);
    }

    template<typename T>
    inline T* AllocRequest(labstor::ipc::qid_t qid, uint32_t size) {
        auto &ipc = pid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qid)];
        return ipc->AllocRequest<T>(qid, size);
    }
    template<typename T>
    inline T* AllocRequest(labstor::queue_pair *qp, uint32_t size) {
        return AllocRequest<T>(qp->GetQID(), size);
    }
    template<typename T>
    inline T* AllocRequest(labstor::queue_pair *qp) {
        return AllocRequest<T>(qp->GetQID(), sizeof(T));
    }

    template<typename T>
    inline void FreeRequest(labstor::ipc::qid_t qid, T *rq) {
        auto &ipc = pid_to_ipc_[LABSTOR_GET_QP_IPC_ID(qid)];
        ipc->FreeRequest<T>(qid, rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::ipc::qtok_t &qtok, T *rq) {
        FreeRequest(qtok.qid_, rq);
    }
    template<typename T>
    inline void FreeRequest(labstor::queue_pair *qp, T *rq) {
        FreeRequest(qp->GetQID(), rq);
    }

    template<typename T>
    inline T* Wait(labstor::ipc::qtok_t &qtok) {
        T *rq;
        labstor::queue_pair *qp;
        GetQueuePair(qp, qtok);
        rq = qp->Wait<T>(qtok.req_id_);
        return rq;
    }

    std::unordered_map<uint32_t,PerProcessIPC*>& GetAllIPC() {
        return pid_to_ipc_;
    }
    inline PerProcessIPC* GetIPC(int pid) {
        return pid_to_ipc_[pid];
    }
};

}
#endif //LABSTOR_SERVER_IPCMANAGER_H
