//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_CLIENT_IPCMANAGER_H
#define LABSTOR_CLIENT_IPCMANAGER_H


#include <thread>
#include <vector>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/types/socket.h>
#include <labstor/constants/constants.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include "labstor/userspace/types/queue_pool.h"
#include "labstor/userspace/types/memory_manager.h"
#include <labstor/types/thread_local.h>
#include <sys/sysinfo.h>
#include <sched.h>
#include <mutex>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

namespace labstor::Client {

class IPCManager : public QueuePool, public MemoryManager {
private:
    int pid_, n_cpu_;
    UnixSocket serversock_;
    bool is_connected_;
public:
    IPCManager() : is_connected_(false) {
        n_cpu_ = get_nprocs_conf();
    }
    void Connect();
    bool IsConnected() {
        return is_connected_;
    }
    inline int GetPID() {
        return pid_;
    }
    inline int GetNumCPU() {
        return n_cpu_;
    }
    inline void GetQueuePair(labstor::queue_pair *&qp, labstor_qid_type_t type, labstor_qid_flags_t flags) {
        AUTO_TRACE("")
        int off = labstor::queue_pair::GetQIDOff(type, flags, labstor::ThreadLocal::GetTid(), GetNumQueuePairsFast(type, flags), pid_);
        QueuePool::GetQueuePair(qp, type, flags, off);
    }
    inline void GetQueuePair(labstor::queue_pair *&qp, labstor_qid_flags_t flags) {
        AUTO_TRACE("")
        int off = labstor::queue_pair::GetQIDOff(0, flags, labstor::ThreadLocal::GetTid(), GetNumQueuePairsFast(0, flags), pid_);
        QueuePool::GetQueuePair(qp, 0, flags, off);
    }
    inline void GetQueuePairByName(labstor::queue_pair *&qp, labstor_qid_flags_t flags, const std::string &str, uint32_t ns_id) {
        AUTO_TRACE("")
        int off = labstor::queue_pair::GetQIDOff(0, flags, str, ns_id, GetNumQueuePairsFast(0, flags), pid_);
        QueuePool::GetQueuePair(qp, 0, flags, off);
    }

    template<typename T=labstor::ipc::request>
    T* Wait(labstor::ipc::qtok_t &qtok) {
        AUTO_TRACE("")
        T *rq;
        labstor::queue_pair *qp;
        QueuePool::GetQueuePair(qp, qtok);
        rq = qp->Wait<T>(qtok.req_id_);
        return rq;
    }
    template<typename T=labstor::ipc::request>
    int WaitFree(labstor::ipc::qtok_t &qtok) {
        AUTO_TRACE("")
        int ret;
        T *rq;
        labstor::queue_pair *qp;
        QueuePool::GetQueuePair(qp, qtok);
        rq = qp->Wait<T>(qtok.req_id_);
        ret = rq->GetCode();
        FreeRequest<T>(qtok, rq);
        return ret;
    }
    template<typename T=labstor::ipc::request>
    int Wait(labstor::ipc::qtok_t *qtoks, int num_qtoks) {
        AUTO_TRACE("num_qtoks", num_qtoks)
        for(int i = 0; i < num_qtoks; ++i) {
            T *rq = Wait<T>(qtoks[i]);
        }
        return LABSTOR_REQUEST_SUCCESS;
    }
    template<typename T=labstor::ipc::request>
    int WaitFree(labstor::ipc::qtok_t *qtoks, int num_qtoks) {
        AUTO_TRACE("num_qtoks", num_qtoks)
        for(int i = 0; i < num_qtoks; ++i) {
            T *rq = Wait<T>(qtoks[i]);
            //TODO: Check if request successful
            FreeRequest(qtoks[i], rq);
        }
        return LABSTOR_REQUEST_SUCCESS;
    }

    void PauseQueues();
    void WaitForPause();
    void ResumeQueues();
private:
    void CreateQueuesSHMEM(int num_queues, int queue_size);
    void CreatePrivateQueues(int num_queues, int queue_size);
};

}
#endif //LABSTOR_CLIENT_IPCMANAGER_H
