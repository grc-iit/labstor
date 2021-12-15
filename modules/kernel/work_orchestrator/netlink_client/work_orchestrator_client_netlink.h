//
// Created by lukemartinlogan on 9/17/21.
//

#ifndef LABSTOR_WORKER_CLIENT_NETLINK_H
#define LABSTOR_WORKER_CLIENT_NETLINK_H

#include <labstor/types/daemon.h>
#include <labstor/kernel/client/macros.h>
#include <labstor/kernel/client/kernel_client.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <labstor/types/data_structures/spsc/shmem_work_queue.h>

namespace labstor::kernel::netlink {

class WorkerClient : public labstor::Daemon {
private:
    int num_workers_;
    int worker_id_;
    LABSTOR_KERNEL_CLIENT_T kernel_client_;
public:
    WorkerClient() = default;
    WorkerClient(int worker_id) : worker_id_(worker_id) {
        kernel_client_ = LABSTOR_KERNEL_CLIENT;
    }

    int CreateWorkers(int num_workers, int region_id, size_t region_size, size_t time_slice_us);
    void AssignQueuePair(labstor::ipc::queue_pair *qp, labstor::ipc::queue_pair *qp_kern, labstor::ipc::queue_pair_ptr &ptr);

    void Start();
    void Pause();
    void Resume();
    void Wait();
    void Stop();
    void SetAffinity(int cpu_id);
};

}

#endif //LABSTOR_WORKER_CLIENT_NETLINK_H
