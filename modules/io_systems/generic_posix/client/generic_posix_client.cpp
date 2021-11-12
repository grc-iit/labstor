//
// Created by lukemartinlogan on 8/20/21.
//

#include "posix_client.h"
#include <generic_posix/posix.h>
#include <labstor/userspace_client/client.h>

class GenericFS_Client {
private:
    std::unordered_map<int, int> fds_; //A table to manage the set of fds

public:
    static int Open(const char *pathname, int flags, mode_t mode) {
        int pathlen = strlen(pathname);
        auto ipc_manager_ = scs::Singleton<labstor::Client::IPCManager>::GetInstance();
        if(!ipc_manager_->IsConnected()) {
            ipc_manager_->Connect();
        }

        struct posix_request *rq = (struct posix_request *)ipc_manager->Alloc(sizeof(struct posix_request) + pathlen, tid);
        rq->open.path = pathname;
        rq->open.flags = flags;
        rq->open.mode = mode;

        labstor::queue_pair &qp = ipc_manager_->GetQueue(tid);
        uint32_t qtok = qp->StartRequest();
        qp->Wait(qtok);
    }

    static void Write(int fd, const void *buf, size_t count) {
    }
};

