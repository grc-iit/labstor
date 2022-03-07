//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_IPC_TEST_H
#define LABSTOR_IPC_TEST_H

#define IPC_TEST_MODULE_ID "IPCTest"

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_poll.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
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

struct labstor_ipc_test_request {
    struct labstor_request header_;
    int nonce_;
#ifdef __cplusplus
    inline void IPCClientStart(int ns_id, int nonce) {
        header_.ns_id_ = ns_id;
        header_.op_ = static_cast<int>(labstor::IPCTest::Ops::kStartIPCTest);
        nonce_ = nonce;
    }
    inline void IPCKernelStart(int ns_id) {
        header_.ns_id_ = ns_id;
        header_.op_ = static_cast<int>(labstor::IPCTest::Ops::kStartIPCTest);
    }
    inline void Copy(labstor_ipc_test_request *rq) {
        header_.SetCode(rq->header_.GetCode());
        nonce_ = rq->nonce_;
    }
    int GetReturnCode() {
        return header_.code_;
    }
#endif
};

#ifdef __cplusplus
struct labstor_poll_ipc_test_request : public labstor::ipc::poll_request_single<labstor_ipc_test_request> {
    void Init(labstor::queue_pair *qp, labstor_ipc_test_request *reply_rq, labstor::ipc::qtok_t &poll_qtok) {
        int op = static_cast<int>(labstor::IPCTest::Ops::kCompleteIPCTest);
        labstor::ipc::poll_request_single<labstor_ipc_test_request>::Init(qp, reply_rq, poll_qtok, op);
    }
};
#endif


#endif //LABSTOR_IPC_TEST_H
