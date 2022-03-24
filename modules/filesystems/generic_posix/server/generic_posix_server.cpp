//
// Created by lukemartinlogan on 8/20/21.
//

#include <cstring>
#include <generic_posix.h>
#include "generic_posix_server.h"

#define PID_FD(pid, fd) (((uint64_t)pid<<32) + fd)

bool labstor::GenericPosix::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("")
    return true;
}
LABSTOR_MODULE_CONSTRUCT(labstor::GenericPosix::Server, GENERIC_POSIX_MODULE_ID)