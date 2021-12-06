//
// Created by lukemartinlogan on 11/26/21.
//

#include "dummy_server.h"

void labstor::test::Dummy::Server::ProcessRequest(labstor::ipc::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    AUTO_TRACE("labstor::test::Dummy::Server::ProcessRequest", request->op_, request->req_id_)
    switch(static_cast<Ops>(request->op_)) {
        case Ops::kGetValue: {
            dummy_submit_request *rq_submit;
            dummy_complete_request *rq_complete;
            rq_submit = reinterpret_cast<dummy_submit_request*>(request);
            rq_complete = reinterpret_cast<dummy_complete_request*>(ipc_manager_->AllocRequest(qp, sizeof(dummy_complete_request)));
            rq_complete->Init(25);
            qp->Complete(rq_submit, rq_complete);
            ipc_manager_->FreeRequest(qp, reinterpret_cast<labstor::ipc::request*>(rq_submit));
            break;
        }
    }
}