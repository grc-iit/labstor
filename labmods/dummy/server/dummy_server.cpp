//
// Created by lukemartinlogan on 11/26/21.
//

#include "dummy_server.h"

bool labstor::test::Dummy::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("")
    //AUTO_TRACE(request->op_, request->req_id_)
    switch(static_cast<Ops>(request->op_)) {
        case Ops::kInit: {
            return Initialize(qp, request, creds);
        }
        case Ops::kGetValue: {
            dummy_request *rq = reinterpret_cast<dummy_request*>(request);
            rq->Complete(5543);
            qp->Complete(rq);
            return true;
        }
    }
    return true;
}

LABSTOR_MODULE_CONSTRUCT(labstor::test::Dummy::Server, LABSTOR_DUMMY_MODULE_ID)