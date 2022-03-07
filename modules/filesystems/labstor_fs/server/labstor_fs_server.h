//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_LABSTOR_FS_SERVER_H
#define LABSTOR_LABSTOR_FS_SERVER_H


#include <labstor_fs.h>
#include <generic_posix/generic_posix.h>

#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/server.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/module_manager.h>
#include <labstor/userspace/server/ipc_manager.h>
#include <labstor/userspace/server/namespace.h>
#include <labstor/types/data_structures/mpmc/unordered_map/shmem_int_map.h>

namespace labstor::LabFS {
class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
public:
    Server() : labstor::Module(LABFS_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    void ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) override;
    inline void Init(labstor::queue_pair *qp, init_request *client_rq, labstor::credentials *creds);
    inline void Open(labstor::queue_pair *qp, generic_posix_open_request *client_rq, labstor::credentials *creds);
    inline void Close(labstor::queue_pair *qp, generic_posix_close_request *client_rq, labstor::credentials *creds);
    inline void IOStart(labstor::queue_pair *qp, generic_posix_io_request *client_rq, labstor::credentials *creds);
    inline void IOComplete(labstor::queue_pair *qp, generic_posix_io_request *client_rq, labstor::credentials *creds);
};
}

#endif //LABSTOR_LABSTOR_FS_SERVER_H
