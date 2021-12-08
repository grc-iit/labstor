//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_IPC_TEST_H
#define LABSTOR_IPC_TEST_H

#define IPC_TEST_MODULE_ID "IPCTest"

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/types/data_structures/shmem_qtok.h>

enum IPCTestOps {
    LABSTOR_START_IPC_TEST,
    LABSTOR_COMPLETE_IPC_TEST
};

#ifdef __cplusplus
namespace labstor::IPCTest {
    enum class Ops {
        kStartIPCTest,
        kCompleteIPCTest
    };
}
#endif

struct labstor_submit_ipc_test_request {
    struct labstor_request header_;
    int nonce_;
#ifdef __cplusplus
    inline void Init(int ns_id, int nonce) {
        header_.ns_id_ = ns_id;
        header_.op_ = static_cast<int>(labstor::IPCTest::Ops::kStartIPCTest);
        nonce_ = nonce;
    }
    inline void Init(int ns_id) {
        header_.ns_id_ = ns_id;
        header_.op_ = static_cast<int>(labstor::IPCTest::Ops::kStartIPCTest);
    }
    inline void Init(labstor_submit_ipc_test_request *rq) {
        header_.ns_id_ = rq->header_.ns_id_;
        header_.op_ = static_cast<int>(labstor::IPCTest::Ops::kStartIPCTest);
        nonce_ = rq->nonce_;
    }
    int GetReturnCode() {
        return header_.ns_id_;
    }
#endif
};

typedef struct labstor_submit_ipc_test_request labstor_complete_ipc_test_request;

struct labstor_poll_ipc_test_request {
    struct labstor_request header_;
    struct labstor_qtok_t kqtok_;
    struct labstor_qtok_t uqtok_;
#ifdef __cplusplus
    void Init(labstor::ipc::queue_pair *qp, labstor_submit_ipc_test_request *rq, labstor::ipc::qtok_t &qtok) {
        header_.ns_id_ = rq->header_.ns_id_;
        header_.op_ = static_cast<int>(labstor::IPCTest::Ops::kCompleteIPCTest);
        kqtok_ = qtok;
        uqtok_.qid = qp->GetQid();
        uqtok_.req_id = rq->header_.req_id_;
    }
#endif
};


#endif //LABSTOR_IPC_TEST_H
