//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_CLIENT_IPCMANAGER_H
#define LABSTOR_CLIENT_IPCMANAGER_H


#include <vector>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/shmem_request_queue.h>

#include <sys/socket.h>
#include <sys/un.h>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

namespace labstor::Client {

class IPCManager {
private:
    int serverfd_;
    pthread_t admin_thread_;
    labstor::GenericAllocator *shmem_alloc_;
    labstor::GenericAllocator *internal_alloc_;
    std::vector<labstor::queue_pair> qps_by_flags_;
    std::unordered_map<uint64_t, labstor::queue_pair> qps_by_id_flags_;
public:
    void Connect(int num_queues);
    void SendMSG(void *buffer, size_t size);
    void RecvMSG(void *buffer, size_t size);
    inline labstor::queue_pair& GetQueuePair(uint32_t flags, int core) { return qps_by_flags_[flags]; }
    inline labstor::queue_pair& GetQueuePair(char *str, uint32_t runtime_id, uint32_t flags) {
        qps_by_id_flags_[flags | hash];
    }
    void PauseQueues();
    void ResumeQueues();
private:
    void CreateQueuesSHMEM(int num_queues);
    void CreateInternalQueues(int num_queues);
};

}
#endif //LABSTOR_CLIENT_IPCMANAGER_H
