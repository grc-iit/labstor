//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SERVER_IPCMANAGER_H
#define LABSTOR_SERVER_IPCMANAGER_H

#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <mutex>

#include <labstor/types/messages.h>
#include <labstor/types/socket.h>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/allocator.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_int_map.h>
#include <labstor/userspace_server/macros.h>

namespace labstor::Server {

struct PerProcessIPC {
    UnixSocket clisock_;
    labstor::credentials creds_;
    void *shmem_region_;
    labstor::GenericAllocator *alloc_;
    std::vector<labstor::ipc::queue_pair> qps_;

    PerProcessIPC() {}
    PerProcessIPC(int fd, labstor::credentials creds) : clisock_(fd), creds_(creds) {}
    PerProcessIPC(const PerProcessIPC &old_ipc) {
        clisock_ = old_ipc.clisock_;
        creds_ = old_ipc.creds_;
        shmem_region_ = old_ipc.shmem_region_;
        alloc_ = old_ipc.alloc_;
        qps_ = old_ipc.qps_;
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
    std::unordered_map<int32_t, PerProcessIPC> pid_to_ipc_;
    labstor::ipc::int_map<int, uint32_t> pid_to_num_stream_qps_;
    labstor::ipc::int_map<uint32_t, labstor::ipc::queue_pair> qps_by_id_;
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
public:
    IPCManager() {
        pid_ = getpid();
        labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    }
    ~IPCManager() {}

    inline void SetServerFd(int fd) { server_fd_ = fd; }
    inline int GetServerFd() { return server_fd_; }

    void RegisterClient(int client_fd, labstor::credentials &creds);
    void *Allocate(int size, bool internal);
    void Free(void *ptr);
    void RegisterQP(PerProcessIPC client_ipc, labstor::ipc::admin_request header);
    void PauseQueues();
    void WaitForPause();
    void ResumeQueues();

    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_num_stream_qps_[pid_];
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, sched_getcpu(), num_qps, pid_)];
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, int hash) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_num_stream_qps_[pid_];
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, hash, num_qps, pid_)];
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, const std::string &str, uint32_t ns_id) {
        if(LABSTOR_QP_IS_STREAM(flags)) {
            uint32_t num_qps = pid_to_num_stream_qps_[pid_];
            qp = qps_by_id_[labstor::ipc::queue_pair::GetStreamQueuePairID(flags, str, ns_id, num_qps, pid_)];
            return;
        }
    }
    inline void GetQueuePair(labstor::ipc::queue_pair &qp, uint32_t flags, uint32_t depth=0, int pid=-1) {
        if(pid >= 0) {
            uint32_t num_qps = pid_to_num_stream_qps_[pid_];
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

    std::unordered_map<int, PerProcessIPC>& GetIPCTable() {
        return pid_to_ipc_;
    }
};

}
#endif //LABSTOR_SERVER_IPCMANAGER_H
