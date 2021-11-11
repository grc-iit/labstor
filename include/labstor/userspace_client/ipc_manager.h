//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_CLIENT_IPCMANAGER_H
#define LABSTOR_CLIENT_IPCMANAGER_H


#include <vector>
#include <labstor/types/basics.h>
#include <labstor/ipc/request_queue.h>

#include <sys/socket.h>
#include <sys/un.h>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

namespace labstor::Client {

class IPCManager {
private:
    int serverfd_;
    pthread_t admin_thread_;
    void *interal_qp_region_;
    std::vector<labstor::ipc::queue_pair> shmem_qps_;
    std::vector<labstor::ipc::queue_pair> internal_qps_;
public:
    void Connect(int num_queues);
    void SendMSG(void *buffer, size_t size);
    void RecvMSG(void *buffer, size_t size);
    labstor::ipc::queue_pair& GetQueuePair(int flags, int core) { return shmem_qps_[core]; }
    void PauseQueues();
    void ResumeQueues();
private:
    void CreateQueuesSHMEM(int num_queues);
    void CreateInternalQueues(int num_queues);
};

}
#endif //LABSTOR_CLIENT_IPCMANAGER_H
