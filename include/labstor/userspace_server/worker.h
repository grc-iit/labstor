//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SERVER_WORKER_H
#define LABSTOR_SERVER_WORKER_H

#ifdef __cplusplus

#include <thread>
#include <labstor/userspace_server/macros.h>
#include <labstor/userspace_server/namespace.h>
#include <labstor/types/daemon/daemon.h>
#include <labstor/types/data_structures/shmem_work_queue.h>

namespace labstor::Server {

class Worker : public DaemonWorker {
private:
    LABSTOR_NAMESPACE_T namespace_;
    void *region_;
    labstor::ipc::work_queue work_queue_;
public:
    Worker(uint32_t depth) {
        namespace_ = LABSTOR_NAMESPACE;
        uint32_t region_size = labstor::ipc::work_queue::GetSize(depth);
        region_ = malloc(region_size);
        work_queue_.Init(region_, region_size);
    }
    void AssignQP(labstor::ipc::queue_pair &qp, void *base) {
        work_queue_.Enqueue(qp, base);
    }
    void DoWork();
};

}

#endif

#endif //LABSTOR_SERVER_WORKER_H
