//
// Created by lukemartinlogan on 9/11/21.
//

#ifndef LABSTOR_CLIENT_H
#define LABSTOR_CLIENT_H

#include <pthread.h>
#include <memory>
#include <vector>

#include <labstor/types/module.h>
#include <labstor/types/ipc_pool.h>

namespace labstor {

class LabStorClientContext {
private:
    std::vector<pthread_t> workers_;
    labstor::PerProcessIPC ipc_;
    labstor::ModuleManager pkg_pool_;
public:
    inline bool IsConnected() { return ipc_.IsConnected(); }
    bool Connect(int num_queues, size_t queue_size);
    void UpdateModule(std::string path) { pkg_pool_.UpdateModule(path); }
    labstor::Module *GetModule(labstor::id module_id) { return pkg_pool_.GetModule(module_id); }
    labstor::ipc::queue_pair& GetQueuePair(int i) { return ipc_.GetQueuePair(i); }
    int GetNumQueuePairs() { return ipc_.GetNumQueues(); }
    int GetSocket() { return ipc_.GetSocket(); }
};

}

#endif //LABSTOR_CLIENT_H
