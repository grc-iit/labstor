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

namespace labstor::GenericPosix {
class Server : public labstor::Module {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    LABSTOR_NAMESPACE_T namespace_;
public:
    Server() : labstor::Module(GENERIC_POSIX_MODULE_ID) {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        namespace_ = LABSTOR_NAMESPACE;
    }
    void ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) override;
    inline int PriorSlash(char *path, int len);
    inline void Open(labstor::ipc::queue_pair *qp, generic_posix_open_request *client_rq, labstor::credentials *creds);
    inline void Close(labstor::ipc::queue_pair *qp, generic_posix_close_request *client_rq, labstor::credentials *creds);
    inline void IOStart(labstor::ipc::queue_pair *qp, generic_posix_io_request *client_rq, labstor::credentials *creds);
    inline void IOComplete(labstor::ipc::queue_pair *qp, generic_posix_poll_io_request *client_rq, labstor::credentials *creds);
};
}
#endif //LABSTOR_GENERIC_POSIX_SERVER_H
