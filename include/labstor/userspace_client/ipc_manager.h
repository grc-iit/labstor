//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_CLIENT_IPCMANAGER_H
#define LABSTOR_CLIENT_IPCMANAGER_H


#include <vector>
#include <labstor/types/socket.h>
#include <labstor/types/basics.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/shmem_request_queue.h>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

/*
 * We do not need qtokens to index queues; they are just ids.
 * We need qids to be unique.
 * Qids should be allocated using a combination
 *
 * Queue Index Properties:
 * [flags | hash(str, runtime_id)] #For keying things to same file
 * [flags | core] #For scalable, low-latency request submission
 * [flags | depth | refcnt] #For batch submissions
 * */

namespace labstor::Client {

class IPCManager {
private:
    UnixSocket serversock_;
    labstor::GenericAllocator *shmem_alloc_;
    labstor::GenericAllocator *internal_alloc_;
    std::vector<labstor::queue_pair> qps_;
    std::unordered_map<uint64_t, labstor::queue_pair> qps_by_id_;
public:
    void Connect(int num_queues);
    inline labstor::queue_pair& GetQueuePair(uint32_t flags) {
    }
    inline labstor::queue_pair& GetQueuePair(uint32_t flags, const std::string &str, uint32_t runtime_id) {
    }
    inline labstor::queue_pair& GetQueuePair(uint32_t flags, uint32_t depth) {
    }
    void PauseQueues();
    void ResumeQueues();
private:
    void CreateQueuesSHMEM(int num_queues);
    void CreateInternalQueues(int num_queues);
};

}
#endif //LABSTOR_CLIENT_IPCMANAGER_H
