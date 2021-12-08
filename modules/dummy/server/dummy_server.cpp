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
            rq_complete = ipc_manager_->AllocRequest<dummy_complete_request>(qp);
            TRACEPOINT("labstor::test::Dummy::Server", "CompleteRequestID", request->req_id_,
                       ((size_t)rq_complete - (size_t)ipc_manager_->GetRegion(creds->pid)));
            rq_complete->Init(5543);
            TRACEPOINT("labstor::test::Dummy::Server", "Starting completion")
            qp->Complete(rq_submit, rq_complete);
            ipc_manager_->FreeRequest<dummy_submit_request>(qp, rq_submit);
            break;
        }
    }
}