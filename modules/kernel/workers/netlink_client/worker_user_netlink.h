//
// Created by lukemartinlogan on 9/17/21.
//

#ifndef LABSTOR_WORKER_USER_NETLINK_H
#define LABSTOR_WORKER_USER_NETLINK_H

#include <labstor/kernel_client/macros.h>
#include <labstor/kernel_client/kernel_client.h>

class WorkerNetlinkClient {
private:
    LABSTOR_KERNEL_CLIENT_T kernel_client_;
public:
    WorkerNetlinkClient() {
        kernel_client_ = scs::Singleton<labstor::Kernel::NetlinkClient>::GetInstance();
    }
    int CreateWorkers(int num_workers, int region_id, size_t region_size, size_t time_slice_us);
    int SetAffinity(int worker_id, int cpu_id);
};

#endif //LABSTOR_WORKER_USER_NETLINK_H
