//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_GENERIC_POSIX_SERVER_H
#define LABSTOR_GENERIC_POSIX_SERVER_H

#include <generic_posix.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>
#include <labstor/types/data_structures/mpmc/unordered_map/shmem_int_map.h>

namespace labstor::GenericPosix {
class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
    labstor::GenericAllocator *inode_alloc_;
public:
    Server() : labstor::Module(GENERIC_POSIX_MODULE_ID) {
        void *region = malloc(1<<20);
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    void ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) override;
    void Initialize(labstor::ipc::request *rq) {}
};
}
#endif //LABSTOR_GENERIC_POSIX_SERVER_H
