//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <labmods/registrar/registrar.h>

#include "no_op_server.h"

void labstor::iosched::NoOp::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE(request->op_, request->req_id_)
    switch (static_cast<Ops>(request->op_)) {

    }
}

//Need to get number of HW queues
//Key requests to kernel queues

LABSTOR_MODULE_CONSTRUCT(labstor::iosched::NoOp::Server, NO_OP_IOSCHED_MODULE_ID);