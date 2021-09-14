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
        auto labstor_client = Singleton<labstor::LabStorClientContext>::GetInstance();
        if(!labstor_client->IsConnected()) {
            labstor_client->Connect();
        }
        labstor::queue_pair *qp = labstor_client->GetQueue(tid);
        struct posix_request *rq = (struct posix_request *)qp->GetRequest();
        rq->open.path = pathname;
        rq->open.flags = flags;
        rq->open.mode = mode;
        int qtok = qp->StartRequest();
        qp->Wait(qtok);
    }

    static void Write() {

    }
};

