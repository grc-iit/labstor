
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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