//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SPDK_DRIVER_H
#define LABSTOR_SPDK_DRIVER_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_poll.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <modules/registrar/registrar.h>

#define SPDK_MODULE_ID "SPDK"

namespace labstor::iosched::SPDK {
enum class Ops {
    kRead,
    kWrite
};

struct spdk_init_request : labstor::Registrar::register_request {
    int nvme_ns_id_;
    labstor::id traddr_;
    void ConstructModuleStart(const std::string &module_id, const std::string &key, const std::string &traddr, int nvme_ns_id) {
        labstor::Registrar::register_request::ConstructModuleStart(module_id, key);
        traddr_.copy(traddr);
        nvme_ns_id_ = nvme_ns_id;
    }
};

struct spdk_poll_request : labstor::ipc::request {
    labstor::queue_pair *qp_;
};
}

#endif //LABSTOR_SPDK_DRIVER_H
